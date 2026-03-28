//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/spmc_ring_buffer.h"

#include <atomic>
#include <cerrno>
#include <format>
#include <limits>
#include <memory>
#include <system_error>
#include <type_traits>

#include <fcntl.h>        // AT_FDCWD
#include <sys/syscall.h>  // SYS_renameat2
#include <unistd.h>       // syscall()

#include "grape/exception.h"
#include "grape/shared_memory.h"

namespace {
//-------------------------------------------------------------------------------------------------
auto shmName(std::string_view name) -> std::string {
  return name.starts_with('/') ? std::string(name) : std::format("/{}", name);
}

//-------------------------------------------------------------------------------------------------
auto shmStagingName(std::string_view name) -> std::string {
  return std::format("{}.__init", shmName(name));
}

//-------------------------------------------------------------------------------------------------
auto shmRename(const std::string& from, const std::string& to)
    -> std::expected<void, grape::Error> {
  static constexpr auto SHM_FS_ROOT = "/dev/shm";
  const auto from_path = std::format("{}{}", SHM_FS_ROOT, from);
  const auto to_path = std::format("{}{}", SHM_FS_ROOT, to);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::syscall(SYS_renameat2, AT_FDCWD, from_path.c_str(), AT_FDCWD, to_path.c_str(),
                RENAME_NOREPLACE) == -1) {
    const auto err = errno;
    if (err == EEXIST) {
      return std::unexpected{ grape::Error{ std::format("'{}' already exists", to) } };
    }
    return std::unexpected(
        grape::Error{ "(renameat2) ", std::error_code(err, std::system_category()).message() });
  }
  return {};
}

//=================================================================================================
struct Control {
  std::uint64_t magic{};
  std::uint64_t write_count{};
  grape::spmc_ring_buffer::Config config;
  static constexpr auto MAGIC = 0x00465542434D5053U;  //!< "SPMCBUF\0";
};

static_assert(std::is_trivially_copyable_v<Control>);
static_assert(alignof(Control) <= alignof(std::max_align_t));

// Control::write_count will be atomically referenced in writer and reader. For this to work:
static_assert(std::atomic_ref<std::uint64_t>::is_always_lock_free);
static_assert(alignof(std::uint64_t) >= std::atomic_ref<std::uint64_t>::required_alignment);

}  // namespace

namespace grape::spmc_ring_buffer {

//=================================================================================================
// NOLINTNEXTLINE(misc-use-internal-linkage)
class ShmImpl {
public:
  explicit ShmImpl(SharedMemory shm) : shm_(std::move(shm)) {
  }

  [[nodiscard]] auto layout() -> std::pair<Control&, std::span<std::byte>> {
    // TODO: Replace with std::start_lifetime_as (C++23) when available.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return { *reinterpret_cast<Control*>(shm_.data().data()),
             shm_.data().subspan(sizeof(Control)) };
  }

  [[nodiscard]] auto layout() const -> std::pair<const Control&, std::span<const std::byte>> {
    // TODO: Replace with std::start_lifetime_as (C++23) when available.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return { *reinterpret_cast<const Control*>(shm_.data().data()),
             shm_.data().subspan(sizeof(Control)) };
  }

  ~ShmImpl() {
    shm_.close();
  }

  ShmImpl(ShmImpl&& other) = delete;
  auto operator=(ShmImpl&& other) = delete;
  ShmImpl(const ShmImpl&) = delete;
  auto operator=(const ShmImpl&) = delete;

private:
  SharedMemory shm_;
};

//=================================================================================================
struct Writer::Impl : public ShmImpl {
  explicit Impl(SharedMemory shm) : ShmImpl(std::move(shm)) {
  }
};

//-------------------------------------------------------------------------------------------------
auto Writer::create(std::string_view name, const Config& config) -> std::expected<Writer, Error> {
  if (config.frame_length == 0 || config.num_frames == 0) {
    return std::unexpected{ Error{
        "Invalid configuration. Requires non-zero frame length and num frames" } };
  }
  if (config.frame_length > std::numeric_limits<std::size_t>::max() / config.num_frames) {
    return std::unexpected{ Error{
        "Invalid configuration. frame_length * num_frames overflows size_t" } };
  }
  const auto data_size = config.frame_length * config.num_frames;

  // create shared memory region under a staging name
  const auto shm_name = shmName(name);
  const auto staging_name = shmStagingName(name);
  std::ignore = SharedMemory::remove(staging_name);  // clean up stale ones from the past
  const auto shm_len = sizeof(Control) + data_size;
  auto maybe_shm = SharedMemory::create(staging_name, shm_len, SharedMemory::Access::ReadWrite);
  if (not maybe_shm) {
    return std::unexpected{ maybe_shm.error() };
  }

  // initialise the control region
  auto impl = std::make_unique<Writer::Impl>(std::move(maybe_shm.value()));
  auto& control = impl->layout().first;
  control.config = config;
  control.write_count = 0U;
  control.magic = Control::MAGIC;

  // rename and make it available for readers
  const auto maybe_rename = shmRename(staging_name, shm_name);
  if (not maybe_rename) {
    std::ignore = SharedMemory::remove(staging_name);
    return std::unexpected{ maybe_rename.error() };
  }

  return Writer{ std::string{ name }, std::move(impl) };
}

//-------------------------------------------------------------------------------------------------
Writer::~Writer() {
  if (impl_ == nullptr) {
    return;
  }
  try {
    std::ignore = SharedMemory::remove(shmName(name_));
    std::ignore = SharedMemory::remove(shmStagingName(name_));
  } catch (...) {
    grape::Exception::print();
  }
}

//-------------------------------------------------------------------------------------------------
Writer::Writer(std::string name, std::unique_ptr<Impl> impl)
  : name_(std::move(name)), impl_{ std::move(impl) } {
}

//-------------------------------------------------------------------------------------------------
void Writer::visit(const WriterFunc& func) {
  auto [ctrl, data] = impl_->layout();
  const auto frame_len = ctrl.config.frame_length;
  const auto write_count = std::atomic_ref<std::uint64_t>{ ctrl.write_count };
  const auto frame_start =
      (write_count.load(std::memory_order_relaxed) % ctrl.config.num_frames) * frame_len;
  func(data.subspan(frame_start, frame_len));
  write_count.fetch_add(1U, std::memory_order_release);
}

//=================================================================================================
struct Reader::Impl : public ShmImpl {
  explicit Impl(SharedMemory shm) : ShmImpl(std::move(shm)) {
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
  const auto& [control, data] = impl->layout();

  if (control.magic != Control::MAGIC) {
    return std::unexpected{ Error{ std::format("'{}' has incorrect magic", name) } };
  }

  if (data.size_bytes() != control.config.frame_length * control.config.num_frames) {
    return std::unexpected{ Error{ std::format("'{}' has unexpected data size", name) } };
  }
  return Reader{ std::move(impl) };
}

//-------------------------------------------------------------------------------------------------
auto Reader::config() const -> Config {
  return impl_->layout().first.config;
}

//-------------------------------------------------------------------------------------------------
auto Reader::visit(const ReaderFunc& func) -> Status {
  const auto& [ctrl, data] = std::as_const(*impl_).layout();
  const auto capacity = ctrl.config.num_frames;
  // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
  const auto write_count_ref =
      std::atomic_ref<std::uint64_t>{ const_cast<std::uint64_t&>(ctrl.write_count) };
  // NOLINTEND(cppcoreguidelines-pro-type-const-cast)

  // To read a valid frame, read count should be within the following bounds:
  // (write_count - capacity) < read_count < write_count

  // if writer lapped reader, fast-forward read count and report dropped frames
  const auto write_count = write_count_ref.load(std::memory_order_acquire);
  const auto read_lag = write_count - read_count_;
  const auto frame_good = ((read_lag < capacity) and (read_count_ < write_count));
  if (not frame_good) {
    read_count_ = write_count;
    return (read_lag == 0UL) ? Status::Empty : Status::Dropped;
  }

  const auto frame_len = ctrl.config.frame_length;
  const auto frame_start = ((read_count_ % capacity) * frame_len);
  func(data.subspan(frame_start, frame_len));

  // if writer lapped reader while processing this frame, invalidate the read
  std::atomic_thread_fence(std::memory_order_seq_cst);
  const auto write_count_post_read = write_count_ref.load(std::memory_order_relaxed);
  const auto post_read_lag = (write_count_post_read - read_count_);
  if (post_read_lag >= capacity) {
    read_count_ = write_count_post_read;
    return Status::Dropped;
  }
  read_count_++;
  return Status::Ok;
}

//-------------------------------------------------------------------------------------------------
Reader::Reader(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {
}

//-------------------------------------------------------------------------------------------------
Reader::~Reader() = default;

//-------------------------------------------------------------------------------------------------
auto Reader::exists(std::string_view name) -> bool {
  return SharedMemory::exists(shmName(name));
}

}  // namespace grape::spmc_ring_buffer
