# drake - data recording and playback

## Design considerations

- `DrakeFile`: Simple core API for file RW
```C++
class DrakeFile{
public:
    struct Event{
        std::uint64_t topic_id;
        std::uint64_t source_id;
        std::uint64_t timestamp;
        std::span<std::size_t> data;
    };
    DrakeFile(file_name, file_open_mode);
    auto Writer::writeNext(const Event&) -> bool;
    auto Reader::readNext() const -> const Event&
    auto Reader::seek(seq_num) -> bool;
    auto Reader::seek(timestamp) -> bool;
};

```
- `DrakeRecorder`: API for recording
  - Takes a list of topics to subscribe to, and creates a `std::vector<ipc::RawSubscriber>`. 
  - An `std::map<std::uint64_t, std::string>` captures `topic_id` -> `topic_name` mapping, where 
  `topic_id` is index into subscriber vector
  - Recorder maintains a large raw memory buffer for staging
  - On subscriber data callback, copy current `wptr` (`wtpr_copy`) into staging buffer, atomically increment `wptr` by data size, write into staging buffer at `wptr_copy`
  - In the background, empty the staging buffer one `Event` at a time by reading `rptr` and write into file   
- `DrakePlayer`: API for playback
  - Instantiates a publisher per topic and per source on its own thread
  - Reads a chunk into staging buffer (single-producer-multi-consumer queue)
  - Each publisher sequencially publishes data as close to the original data rate by independently reading off the staging buffer.

## TODO

- [ ] Research techniques and formats for writing variable-sized records fast. Study LCM, RRD, MCAP
- [ ] Research techniques for reading variable sized records 
- [ ] Implement `DrakeFile`
- [ ] Implement `DrakeRecorder`
- [ ] Implement `DrakePlayer`

## Research

Assumptions: Multiple 1khz sources, each source generates ordered timeseries data, and we want strict global ordering on disk

Do the following in the Writer:
- Have a single writer thread
- Provide an append-only buffer per source(O(1))
- At low-watermark or epoch, flush all buffers using k-way merge using min-heap
- Periodically commit large buffered sequential writes to disk
- Use a deterministic tie-breaker for records with equal timestamps (based on timestamp comparison followed by source_id comparison, let's say)

On the reader, we could read using k-way approach as well, if we store data from each source in its own buffer