//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/spmcq.h"

#include <algorithm>  // for __copy, copy
#include <atomic>
#include <format>
#include <memory>
#include <new>
#include <tuple>  // for __ignore_type, ignore
#include <type_traits>

#include "grape/exception.h"
#include "grape/shared_memory.h"

namespace {
//-------------------------------------------------------------------------------------------------
auto shmName(std::string_view name) -> std::string {
  return name.starts_with('/') ? std::string(name) : std::format("/{}", name);
}

//-------------------------------------------------------------------------------------------------
// Aligns 'n' up to the nearest multiple of 'align'
[[nodiscard]] constexpr auto alignUp(std::size_t n, std::size_t align) -> std::size_t {
  return (n + align - 1U) & ~(align - 1U);
}

//=================================================================================================
// Memory layout of the shared memory region:
// [Control | Metadata (optional) | padding (aligns Frames with cache-line) | Frame 0 ... Frame N-1]
//
// control_offset = 0
// metadata_offset = sizeof(Control) [known at compile time]
// frames_offset = alignUp(metadata_offset + metadata_length, cache_line_size)
//=================================================================================================

//=================================================================================================
// Control region. Describes configuration and state of ring buffer
struct Control {
  std::uint64_t write_count{};
  grape::spmcq::Config config;
  std::uint64_t metadata_length{};
  std::uint64_t frames_offset{};
  std::uint64_t magic{};
  static constexpr auto MAGIC = 0x00465542434D5053U;  //!< "SPMCBUF\0";
};

static_assert(offsetof(Control, write_count) == 0U);  // ensure alignment for atomic_ref
static_assert(std::is_trivially_copyable_v<Control>);
static_assert(std::atomic_ref<std::uint64_t>::is_always_lock_free);

}  // namespace

namespace grape::spmcq {

//=================================================================================================
struct Writer::Impl : public SharedMemory {
  explicit Impl(SharedMemory shm) : SharedMemory(std::move(shm)) {
  }
};

//-------------------------------------------------------------------------------------------------
auto Writer::create(std::string_view name, const Config& config,
                    std::span<const std::byte> metadata) -> std::expected<Writer, Error> {
  if (config.frame_length < Config::MIN_FRAME_LENGTH) {
    return std::unexpected{ Error{ "Invalid configuration (frame_length)" } };
  }
  if (config.num_frames < Config::MIN_FRAMES) {
    return std::unexpected{ Error{ "Invalid configuration (num_frames)" } };
  }
  const auto data_size = config.frame_length * config.num_frames;

  const auto shm_name = shmName(name);
  const auto control_len = sizeof(Control);
  const auto metadata_len = metadata.size_bytes();
  const auto preamble_len = control_len + metadata_len;
  const auto frames_offset = alignUp(preamble_len, std::hardware_destructive_interference_size);
  const auto shm_len = frames_offset + data_size;
  auto maybe_shm = SharedMemory::create(shm_name, shm_len, SharedMemory::Access::ReadWrite);
  if (not maybe_shm) {
    if (SharedMemory::exists(shm_name)) {
      return std::unexpected{ Error{ std::format("'{}' already exists", shm_name) } };
    }
    return std::unexpected{ maybe_shm.error() };
  }

  // initialise the shared memory region
  auto impl = std::make_unique<Writer::Impl>(std::move(maybe_shm.value()));
  auto shm = impl->data();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* ctrl = reinterpret_cast<Control*>(shm.data());

  ctrl->magic = 0U;  // Will be set when the region is ready for readers
  ctrl->config = config;
  ctrl->write_count = 0U;
  ctrl->metadata_length = metadata_len;
  ctrl->frames_offset = frames_offset;

  // copy metadata into the buffer immediately after the control block
  std::ranges::copy(metadata, shm.subspan(control_len).begin());

  // Mark as ready for readers
  std::atomic_ref<std::uint64_t>(ctrl->magic).store(Control::MAGIC, std::memory_order_release);

  // cache pointers to hot-path data
  auto* const write_count_ptr = &ctrl->write_count;
  const auto frames = shm.subspan(frames_offset);

  return Writer{ std::string{ name }, std::move(impl), config, frames, write_count_ptr };
}

//-------------------------------------------------------------------------------------------------
Writer::~Writer() {
  if (impl_ == nullptr) {
    return;  // moved-from state, nothing to clean up
  }
  try {
    std::ignore = SharedMemory::remove(shmName(name_));
  } catch (...) {
    grape::Exception::print();
  }
}

//-------------------------------------------------------------------------------------------------
Writer::Writer(std::string name, std::unique_ptr<Impl> impl, const Config& config,
               std::span<std::byte> frames, std::uint64_t* write_count_ptr)
  : name_(std::move(name))
  , impl_{ std::move(impl) }
  , config_{ config }
  , frames_{ frames }
  , write_count_ptr_{ write_count_ptr } {
}

//=================================================================================================
struct Reader::Impl : public SharedMemory {
  explicit Impl(SharedMemory shm) : SharedMemory(std::move(shm)) {
  }
};

//-------------------------------------------------------------------------------------------------
auto Reader::connect(std::string_view name) -> std::expected<Reader, Error> {
  auto maybe_shm = SharedMemory::open(shmName(name), SharedMemory::Access::ReadOnly);
  if (not maybe_shm) {
    return std::unexpected{ maybe_shm.error() };
  }

  if (maybe_shm->data().size_bytes() < sizeof(Control)) {
    return std::unexpected{ Error{ std::format("'{}' has unexpected size (too small)", name) } };
  }

  auto impl = std::make_unique<Reader::Impl>(std::move(maybe_shm.value()));
  auto shm = impl->data();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* ctrl = reinterpret_cast<Control*>(shm.data());

  const auto magic = std::atomic_ref<std::uint64_t>(ctrl->magic).load(std::memory_order_acquire);
  if (magic != Control::MAGIC) {
    return std::unexpected{ Error{ std::format("'{}' is not ready", name) } };
  }
  const auto& config = ctrl->config;
  const auto frames_len = config.frame_length * config.num_frames;
  const auto expected_size = ctrl->frames_offset + frames_len;
  if (shm.size_bytes() < expected_size) {
    return std::unexpected{ Error{
        std::format("'{}' has unexpected data size (too small)", name) } };
  }

  const auto control_len = sizeof(Control);
  if (control_len + ctrl->metadata_length > ctrl->frames_offset) {
    return std::unexpected{ Error{ std::format("'{}' has invalid metadata length", name) } };
  }
  const auto metadata = shm.subspan(control_len, ctrl->metadata_length);

  auto* const write_count_ptr = &ctrl->write_count;
  const auto frames = shm.subspan(ctrl->frames_offset, frames_len);

  return Reader{ std::move(impl), config, metadata, frames, write_count_ptr };
}

//-------------------------------------------------------------------------------------------------
auto Reader::config() const -> Config {
  return config_;
}

//-------------------------------------------------------------------------------------------------
auto Reader::metadata() const -> std::span<const std::byte> {
  return metadata_;
}

//-------------------------------------------------------------------------------------------------
Reader::Reader(std::unique_ptr<Impl> impl, const Config& config,
               // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
               std::span<const std::byte> metadata, std::span<const std::byte> frames,
               const std::uint64_t* write_count_ptr)
  : impl_{ std::move(impl) }
  , config_{ config }
  , metadata_{ metadata }
  , frames_{ frames }
  , write_count_ptr_{ write_count_ptr } {
}

//-------------------------------------------------------------------------------------------------
Reader::~Reader() = default;

//-------------------------------------------------------------------------------------------------
auto Reader::exists(std::string_view name) -> bool {
  return SharedMemory::exists(shmName(name));
}

}  // namespace grape::spmcq
