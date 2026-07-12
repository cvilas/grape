# README: Audio 

Cross platform audio support

## Design considerations

- [SDL3](https://github.com/libsdl-org/SDL/)/[SDL3_Mixer](https://wiki.libsdl.org/SDL3_mixer/FrontPage) for platform-independent device handling
- Capture and processing stages implemented following a pipeline architecture
- Support for multi-channel audio
- Low-latency transport over local network
- Facilities to synchronise with camera capture pipeline (without strong coupling across pipelines).

## High-Level Architecture

- Pipeline: capture (N channels) -> encode -> publish (QoS: `BestEffort`, topic: `/hostname/audio`)
- A/V sync policy: 
  - Audio-led (audio playout is the master timeline). 
    - Rationale: keeps the transport simple and avoids coupling sinks that do not need sync. 
    - Note: Audio and video sources could be on different hosts. Sync at sink.
    - Assume PTP sync across such hosts and monotonic clocks
    - Audio sink consumes chunks at hardware playout rate
    - Synchronizer maps desired video presentation time to nearest eligible frame
  - Sync policy
    - Hold early video frames until target presentation time
    - Drop late video frames beyond configured lateness threshold
    - Prefer freshest frame within target window

## 7. Audio Data Model (Multi-Channel)

Audio chunk header fields:

- sample_rate_hz (e.g. 48000)
- channel_count (e.g. 1, 2, 4, 6, 8)
- sample_format (PCM_S16LE, PCM_F32LE)
- channel_layout (explicit layout enum/bitmask; not only count)
- interleaving (v1: interleaved)
- frames_per_chunk
- payload_bytes

Notes:

- fixed chunk duration is recommended (10 ms initial target)
- explicit channel layout avoids ambiguity for >2 channels
- audio jitter buffer: slightly larger than video
- video buffer: bounded queue with drop policy
- track and expose queue depth and drop counters

## 10. Module/API Layout Proposal

Proposed camera module additions:

- include/grape/camera/audio_frame.h
- include/grape/camera/audio_spec.h
- include/grape/camera/media_header.h
- include/grape/camera/audio_capture.h
- include/grape/camera/audio_sink.h
- include/grape/camera/av_synchronizer.h

Proposed apps:

- apps/audio_pub/audio_pub.cpp
- apps/audio_sub/audio_sub.cpp

## 11. Incremental Delivery Plan

Phase 1: Audio transport baseline

- define audio header/spec and serialization contract
- implement audio_pub and audio_sub
- implement basic audio sink
- validate multi-channel transport and playback

Phase 2: Shared timing contract

- add common media header fields to image and audio payloads
- include sequence and both wall/mono capture timestamps
- add latency instrumentation (capture->publish->receive)

Phase 3: A/V synchronization

- implement AvSynchronizer in combined subscriber path
- audio-master policy, drop/hold thresholds, bounded buffers
- expose sync metrics (A-V skew, drops, queue depths)

Phase 4: Hardening

- tests for header compatibility, channel layouts, sync under jitter/drift
- tuning defaults and config surface

## 12. Configuration Surface (Initial)

Publisher config:

- video_topic
- audio_topic
- audio_chunk_ms
- audio_sample_rate_hz
- audio_sample_format
- audio_channel_layout

Subscriber config:

- enable_av_sync
- av_target_offset_ms
- av_max_video_late_ms
- audio_jitter_buffer_ms
- video_buffer_frames

## 13. Validation and Metrics

Track at runtime:

- capture-to-publish latency (audio/video)
- publish-to-receive latency (audio/video)
- end-to-end latency to sink
- A-V skew distribution
- dropped/late video frames
- audio underflow/overflow events

Success criteria:

- synchronized playout stable under nominal LAN jitter
- bounded queues under sustained operation
- multi-channel mapping preserved end-to-end

## 16. SDL3 Implementation Blueprint

This section provides implementation-oriented C++ interfaces and loop skeletons for SDL3-based capture, transport, and sink components.

### 16.1 Proposed Header Types

```cpp
// media_header.h
namespace grape::camera {

enum class PayloadKind : std::uint8_t { Image = 1, Audio = 2 };

struct MediaHeader {
	std::uint64_t stream_id{};           // stable per source stream
	std::uint64_t sequence{};            // monotonically increasing per stream
	std::int64_t capture_time_wall_ns{}; // WallClock::toNanos(...)
	std::int64_t capture_time_mono_ns{}; // monotonic clock timeline
	PayloadKind payload_kind{};
};

}  // namespace grape::camera
```

```cpp
// audio_spec.h
namespace grape::camera {

enum class SampleFormat : std::uint8_t {
	PcmS16Le = 1,
	PcmF32Le = 2,
};

enum class AudioChannelLayout : std::uint16_t {
	Mono = 1,       // FRONT
	Stereo = 2,     // FL, FR
	Quad = 4,       // FL, FR, BL, BR
	FivePointOne = 6,
	SevenPointOne = 8,
};

struct AudioSpec {
	std::uint32_t sample_rate_hz{ 48000 };
	std::uint16_t channel_count{ 2 };
	SampleFormat sample_format{ SampleFormat::PcmF32Le };
	AudioChannelLayout channel_layout{ AudioChannelLayout::Stereo };
	bool interleaved{ true };
};

}  // namespace grape::camera
```

```cpp
// audio_frame.h
namespace grape::camera {

struct AudioFrameHeader {
	MediaHeader media;
	AudioSpec spec;
	std::uint32_t frames_per_chunk{};
	std::uint32_t payload_bytes{};
};

struct AudioFrame {
	AudioFrameHeader header;
	std::span<const std::byte> samples;  // interleaved PCM payload
};

}  // namespace grape::camera
```

### 16.3 SDL3 Audio Publisher Skeleton

```cpp
class AudioPublisher {
public:
	struct Config {
		std::string topic;
		AudioSpec spec;
		std::uint16_t chunk_ms{ 10 };
	};

	explicit AudioPublisher(const Config& cfg);
	void update();

private:
	auto bytesPerFrame() const -> std::size_t;
	auto chunkBytes() const -> std::size_t;
	void publishChunk(std::span<const std::byte> chunk);

	Config cfg_;
	ipc::RawPublisher publisher_;
	SDL_AudioStream* recording_stream_{ nullptr };
	std::vector<std::byte> buffer_;

	std::uint64_t sequence_{ 0 };
	std::uint64_t sample_cursor_frames_{ 0 };  // total frames emitted so far
	std::int64_t mono_t0_ns_{ 0 };
	std::int64_t wall_t0_ns_{ 0 };
};

AudioPublisher::AudioPublisher(const Config& cfg)
	: cfg_(cfg)
	, publisher_({ .name = cfg.topic }) {
	if (not SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		throw std::runtime_error(SDL_GetError());
	}

	const SDL_AudioSpec desired = {
		.format = SDL_AUDIO_F32LE,
		.channels = static_cast<int>(cfg_.spec.channel_count),
		.freq = static_cast<int>(cfg_.spec.sample_rate_hz),
	};

	recording_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &desired,
																								 nullptr, nullptr);
	if (recording_stream_ == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}

	if (not SDL_ResumeAudioStreamDevice(recording_stream_)) {
		throw std::runtime_error(SDL_GetError());
	}

	mono_t0_ns_ = /* MonoClock::toNanos(MonoClock::now()) */ 0;
	wall_t0_ns_ = WallClock::toNanos(WallClock::now());
	buffer_.resize(chunkBytes() * 4U);
}

void AudioPublisher::update() {
	const auto want = static_cast<int>(chunkBytes());
	while (SDL_GetAudioStreamAvailable(recording_stream_) >= want) {
		auto* dst = reinterpret_cast<Uint8*>(buffer_.data());
		const auto got = SDL_GetAudioStreamData(recording_stream_, dst, want);
		if (got < 0) {
			syslog::Error("SDL_GetAudioStreamData failed: {}", SDL_GetError());
			return;
		}
		if (got != want) {
			return;  // wait for full fixed-size chunk
		}
		publishChunk(std::span<const std::byte>(buffer_.data(), static_cast<std::size_t>(got)));
	}
}

void AudioPublisher::publishChunk(std::span<const std::byte> chunk) {
	const auto frames = static_cast<std::uint32_t>(chunk.size_bytes() / bytesPerFrame());

	const auto capture_mono_ns =
			mono_t0_ns_ + static_cast<std::int64_t>((sample_cursor_frames_ * 1000000000ULL) /
																							cfg_.spec.sample_rate_hz);
	const auto capture_wall_ns =
			wall_t0_ns_ + (capture_mono_ns - mono_t0_ns_);  // same origin mapping at start

	const AudioFrameHeader hdr{
		.media = MediaHeader{ .stream_id = 1,
													.sequence = sequence_++,
													.capture_time_wall_ns = capture_wall_ns,
													.capture_time_mono_ns = capture_mono_ns,
													.payload_kind = PayloadKind::Audio },
		.spec = cfg_.spec,
		.frames_per_chunk = frames,
		.payload_bytes = static_cast<std::uint32_t>(chunk.size_bytes()),
	};

	sample_cursor_frames_ += frames;

	// Serialize [hdr][chunk] and publish via RawPublisher.
}
```

### 16.4 SDL3 Audio Subscriber/Sink Skeleton

```cpp
class AudioSink {
public:
	explicit AudioSink(const AudioSpec& spec);
	void push(const AudioFrame& frame);

private:
	SDL_AudioStream* playback_stream_{ nullptr };
};

AudioSink::AudioSink(const AudioSpec& spec) {
	if (not SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		throw std::runtime_error(SDL_GetError());
	}

	const SDL_AudioSpec desired = {
		.format = SDL_AUDIO_F32LE,
		.channels = static_cast<int>(spec.channel_count),
		.freq = static_cast<int>(spec.sample_rate_hz),
	};

	playback_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired,
																								nullptr, nullptr);
	if (playback_stream_ == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}
	if (not SDL_ResumeAudioStreamDevice(playback_stream_)) {
		throw std::runtime_error(SDL_GetError());
	}
}

void AudioSink::push(const AudioFrame& frame) {
	auto* src = const_cast<void*>(static_cast<const void*>(frame.samples.data()));
	if (not SDL_PutAudioStreamData(playback_stream_, src,
																 static_cast<int>(frame.samples.size_bytes()))) {
		syslog::Error("SDL_PutAudioStreamData failed: {}", SDL_GetError());
	}
}
```

### 16.6 SDL3 Device and Event Handling

Handle at least:

- SDL_EVENT_AUDIO_DEVICE_ADDED
- SDL_EVENT_AUDIO_DEVICE_REMOVED
- SDL_EVENT_CAMERA_DEVICE_APPROVED
- SDL_EVENT_CAMERA_DEVICE_DENIED
- SDL_EVENT_CAMERA_DEVICE_ADDED
- SDL_EVENT_CAMERA_DEVICE_REMOVED

Behavior requirements:

- on removal: stop affected stream, mark pipeline degraded, attempt reopen/backoff
- on default device change: reopen default stream while preserving timeline continuity where possible
- on camera warmup: drop initial unstable frames before publishing

### 16.7 Initial Defaults

- audio_sample_rate_hz = 48000
- audio_sample_format = PCM_F32LE
- audio_chunk_ms = 10
- audio_channel_layout = Stereo
- audio_jitter_buffer_ms = 40
- av_max_video_late_ms = 30

### 16.8 Tests to Add Alongside SDL3 Integration

1. Audio header serialization/deserialization round-trip.
2. Chunk timestamp progression from sample cursor (monotonic and strictly increasing).
3. Multi-channel metadata preservation across pub/sub.
4. Sink underflow/overflow accounting.
5. Synchronizer frame drop/hold policy under synthetic jitter.
