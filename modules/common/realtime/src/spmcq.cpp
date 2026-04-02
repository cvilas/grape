//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/spmcq.h"

#include <atomic>
#include <cerrno>
#include <format>
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
  std::uint64_t write_count{};  // note: first member to ensure 8-byte alignment for atomic_ref
  grape::spmcq::Config config;
  std::uint64_t magic{};
  static constexpr auto MAGIC = 0x00465542434D5053U;  //!< "SPMCBUF\0";
};

static_assert(std::is_trivially_copyable_v<Control>);
static_assert(std::atomic_ref<std::uint64_t>::is_always_lock_free);

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
auto Writer::create(std::string_view name, const Config& config) -> std::expected<Writer, Error> {
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
  const auto shm_len = sizeof(Control) + data_size;
  auto maybe_shm = SharedMemory::create(staging_name, shm_len, SharedMemory::Access::ReadWrite);
  if (not maybe_shm) {
    return std::unexpected{ maybe_shm.error() };
  }

  // initialise the control region before constructing Impl so that cached hot data is correct
  auto impl = std::make_unique<Writer::Impl>(std::move(maybe_shm.value()));
  auto [ctrl, data] = impl->layout();
  ctrl.config = config;
  ctrl.write_count = 0U;
  ctrl.magic = Control::MAGIC;

  // cache pointers to hot-path data
  auto* const write_count_ptr = &ctrl.write_count;
  const auto frames = data;

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
  const auto& [ctrl, data] = impl->layout();

  if (ctrl.magic != Control::MAGIC) {
    return std::unexpected{ Error{ std::format("'{}' has incorrect magic", name) } };
  }

  if (data.size_bytes() != ctrl.config.frame_length * ctrl.config.num_frames) {
    return std::unexpected{ Error{ std::format("'{}' has unexpected data size", name) } };
  }

  // cache pointers to hot-path data
  auto* const write_count_ptr = &ctrl.write_count;
  const auto frames = data;

  return Reader{ std::move(impl), ctrl.config, frames, write_count_ptr };
}

//-------------------------------------------------------------------------------------------------
auto Reader::config() const -> Config {
  return config_;
}

//-------------------------------------------------------------------------------------------------
Reader::Reader(std::unique_ptr<Impl> impl, const Config& config, std::span<const std::byte> frames,
               const std::uint64_t* write_count_ptr)
  : impl_{ std::move(impl) }
  , config_{ config }
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
