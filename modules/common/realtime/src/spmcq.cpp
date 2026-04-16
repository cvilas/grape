//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/spmcq.h"

#include <atomic>
#include <cerrno>
#include <format>
#include <memory>
#include <new>
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
  std::uint64_t write_count{};
  grape::spmcq::Config config;
  std::uint64_t metadata_length{};
  std::uint64_t frames_offset{};  //!< byte offset within buf where frame data starts (cache-line
                                  //!< aligned)
  std::uint64_t magic{};
  static constexpr auto MAGIC = 0x00465542434D5053U;  //!< "SPMCBUF\0";
};

// first member to ensure alignment for atomic_ref
static_assert(offsetof(Control, write_count) == 0U);

static_assert(std::is_trivially_copyable_v<Control>);
static_assert(std::atomic_ref<std::uint64_t>::is_always_lock_free);

[[nodiscard]] constexpr auto alignUp(std::size_t n, std::size_t align) -> std::size_t {
  return (n + align - 1U) & ~(align - 1U);
}

}  // namespace

namespace grape::spmcq {

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

private:
  SharedMemory shm_;
};

//=================================================================================================
struct Writer::Impl : public ShmImpl {
  explicit Impl(SharedMemory shm) : ShmImpl(std::move(shm)) {
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

  // create shared memory region under a staging name
  const auto shm_name = shmName(name);
  const auto staging_name = shmStagingName(name);
  std::ignore = SharedMemory::remove(staging_name);  // clean up stale ones from the past
  // align frames to cache line boundary within buf (buf starts at sizeof(Control) in shm)
  const auto frames_offset = alignUp(sizeof(Control) + metadata.size_bytes(),
                                     std::hardware_destructive_interference_size) -
                             sizeof(Control);
  const auto shm_len = sizeof(Control) + frames_offset + data_size;
  auto maybe_shm = SharedMemory::create(staging_name, shm_len, SharedMemory::Access::ReadWrite);
  if (not maybe_shm) {
    return std::unexpected{ maybe_shm.error() };
  }

  // initialise the control region before constructing Impl so that cached hot data is correct
  auto impl = std::make_unique<Writer::Impl>(std::move(maybe_shm.value()));
  auto [ctrl, buf] = impl->layout();
  ctrl.config = config;
  ctrl.write_count = 0U;
  ctrl.metadata_length = metadata.size_bytes();
  ctrl.frames_offset = frames_offset;
  ctrl.magic = Control::MAGIC;

  // copy metadata into the buffer immediately after the control block
  std::ranges::copy(metadata, buf.begin());

  // cache pointers to hot-path data
  auto* const write_count_ptr = &ctrl.write_count;
  const auto frames = buf.subspan(frames_offset);

  // rename and make it available for readers
  const auto maybe_rename = shmRename(staging_name, shm_name);
  if (not maybe_rename) {
    std::ignore = SharedMemory::remove(staging_name);
    return std::unexpected{ maybe_rename.error() };
  }

  return Writer{ std::string{ name }, std::move(impl), config, frames, write_count_ptr };
}

//-------------------------------------------------------------------------------------------------
Writer::~Writer() {
  if (impl_ == nullptr) {
    return;  // moved-from state, nothing to clean up
  }
  try {
    std::ignore = SharedMemory::remove(shmName(name_));
    std::ignore = SharedMemory::remove(shmStagingName(name_));
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
  const auto& [ctrl, buf] = impl->layout();

  if (ctrl.magic != Control::MAGIC) {
    return std::unexpected{ Error{ std::format("'{}' has incorrect magic", name) } };
  }

  const auto expected_size =
      ctrl.frames_offset + (ctrl.config.frame_length * ctrl.config.num_frames);
  if (buf.size_bytes() != expected_size) {
    return std::unexpected{ Error{ std::format("'{}' has unexpected data size", name) } };
  }

  // cache pointers to hot-path data
  auto* const write_count_ptr = &ctrl.write_count;
  const auto metadata = buf.subspan(0, ctrl.metadata_length);
  const auto frames = buf.subspan(ctrl.frames_offset);

  return Reader{ std::move(impl), ctrl.config, metadata, frames, write_count_ptr };
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

/*
Baseline:
-------------------------------------------------------------------------------
Benchmark                                     Time             CPU   Iterations
-------------------------------------------------------------------------------
bmSpmcqWrite/8/iterations:1000000          13.0 ns         13.0 ns      1000000
bmSpmcqWrite/16/iterations:1000000         13.3 ns         13.3 ns      1000000
bmSpmcqWrite/32/iterations:1000000         13.3 ns         13.3 ns      1000000
bmSpmcqWrite/64/iterations:1000000         13.3 ns         13.3 ns      1000000
bmSpmcqWrite/128/iterations:1000000        16.2 ns         16.2 ns      1000000
bmSpmcqWrite/256/iterations:1000000        16.8 ns         16.8 ns      1000000
bmSpmcqWrite/512/iterations:1000000        20.4 ns         20.4 ns      1000000
bmSpmcqWrite/1024/iterations:1000000       30.4 ns         30.4 ns      1000000
bmSpmcqWrite/2048/iterations:1000000       54.8 ns         54.8 ns      1000000
bmSpmcqRead/8/iterations:1000000           2.63 ns         2.63 ns      1000000
bmSpmcqRead/16/iterations:1000000          3.37 ns         3.37 ns      1000000
bmSpmcqRead/32/iterations:1000000          5.39 ns         5.39 ns      1000000
bmSpmcqRead/64/iterations:1000000          7.87 ns         7.87 ns      1000000
bmSpmcqRead/128/iterations:1000000         13.0 ns         13.0 ns      1000000
bmSpmcqRead/256/iterations:1000000         21.8 ns         21.8 ns      1000000
bmSpmcqRead/512/iterations:1000000         43.3 ns         43.3 ns      1000000
bmSpmcqRead/1024/iterations:1000000        85.0 ns         84.8 ns      1000000
bmSpmcqRead/2048/iterations:1000000         169 ns          169 ns      1000000
*/
