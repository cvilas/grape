//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/shared_memory.h"

#include <system_error>

#include <fcntl.h>     // for O_* constants
#include <sys/mman.h>  // for mmap
#include <sys/stat.h>  // For mode constants
#include <unistd.h>    // for open/close

namespace grape::realtime {

//-------------------------------------------------------------------------------------------------
auto SharedMemory::exists(const std::string& name) -> bool {
  const auto fd = ::shm_open(name.c_str(), O_RDONLY, 0);
  const auto exists = (fd > 0);
  if (exists) {
    ::close(fd);
  }
  // Note: Ignores shm_open failing with EMFILE or ENFILE (per-process or system-wide limit on
  // number of open file descriptors)
  return exists;
}

//-------------------------------------------------------------------------------------------------
auto SharedMemory::remove(const std::string& name) -> std::expected<void, Error> {
  const auto status = ::shm_unlink(name.c_str());
  if (status == 0) {
    return {};
  }
  const auto err = std::error_code(errno, std::system_category());
  return std::unexpected(Error{ "(shm_unlink) ", err.message() });
}

//-------------------------------------------------------------------------------------------------
auto SharedMemory::create(const std::string& name, std::size_t size, Access access)
    -> std::expected<SharedMemory, Error> {
  // Flags to specify shm file creation behaviour:
  // - O_CREAT: create shm path if it does not exist
  // - O_EXCL: raise error if shm path already exists before creation
  // - O_RDWR: must be specified to set shm size (later, using ftruncate())
  static constexpr auto OFLAGS = O_CREAT | O_EXCL | O_RDWR;

  // Flags to specify shm file access modes:
  // - User and group ownership are set from the corresponding effective IDs of the calling process
  // - The read/write/execute permission bits are set according to the low-order 9 bits of MODE,
  // except that those bits set in the process file mode creation mask (see umask(2)) are cleared
  // for the new object. (i.e. effective mode = MODE & ~umask)
  static constexpr auto MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

  // const auto process_mask = umask(0);  // temporarily reset process mode so we can set any mode
  const auto fd = ::shm_open(name.c_str(), OFLAGS, MODE);
  // umask(process_mask);  // reinstate process mode mask
  if (fd == -1) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(shm_open) ", err.message() });
  }

  // set size of shared memory
  if (::ftruncate(fd, static_cast<off_t>(size)) == -1) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(ftruncate) ", err.message() });
  }

  // note the size we actually got
  struct stat stats{};
  if (::fstat(fd, &stats) == -1) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(fstat) ", err.message() });
  }
  size = static_cast<std::size_t>(stats.st_size);

  // Map shared memory to this process address space with the desired memory protection
  /// @note mmap allocates memory as multiples of page size (sysconf(_SC_PAGESIZE)) even if
  /// requested size is less than page_size.
  /// @note The address returned is page aligned and meets the alignment requirement of any data
  /// type on the host
  const auto prot = ((access == Access::ReadWrite) ? (PROT_READ | PROT_WRITE) : PROT_READ);
  const auto offset = 0;
  auto* shm_area = ::mmap(nullptr, size, prot, MAP_SHARED, fd, offset);
  if (shm_area == MAP_FAILED) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(mmap) ", err.message() });
  }
  ::close(fd);  // Not needed anymore

  SharedMemory shm;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  shm.data_ = std::span<std::byte>{ reinterpret_cast<std::byte*>(shm_area), size };
  return shm;
}

//-------------------------------------------------------------------------------------------------
auto SharedMemory::open(const std::string& name, Access access)
    -> std::expected<SharedMemory, Error> {
  // Open existing shared memory object
  const auto oflag = ((access == Access::ReadWrite) ? O_RDWR : O_RDONLY);
  static constexpr auto MODE = 0;  // unused
  const auto fd = ::shm_open(name.c_str(), oflag, MODE);
  if (fd == -1) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(shm_open) ", err.message() });
  }

  // read size of the shared memory object
  struct stat stats{};
  if (::fstat(fd, &stats) == -1) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(fstat) ", err.message() });
  }

  // map shared memory to this process address space with the desired memory protection
  static constexpr auto OFFSET = 0;
  const auto size = static_cast<std::size_t>(stats.st_size);
  const auto prot = ((access == Access::ReadWrite) ? (PROT_READ | PROT_WRITE) : PROT_READ);
  auto* shm_area = ::mmap(nullptr, size, prot, MAP_SHARED, fd, OFFSET);
  if (shm_area == MAP_FAILED) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(mmap) ", err.message() });
  }
  ::close(fd);  // Not needed anymore

  SharedMemory shm;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  shm.data_ = std::span<std::byte>{ reinterpret_cast<std::byte*>(shm_area), size };
  return shm;
}

//-------------------------------------------------------------------------------------------------
void SharedMemory::close() {
  if (data_.empty()) {
    return;
  }
  ::munmap(data_.data(), data_.size_bytes());
  data_ = {};
}

//-------------------------------------------------------------------------------------------------
SharedMemory::~SharedMemory() {
  close();
}

//-------------------------------------------------------------------------------------------------
SharedMemory::SharedMemory(SharedMemory&& other) noexcept : data_(other.data_) {
  other.data_ = {};
}

//-------------------------------------------------------------------------------------------------
auto SharedMemory::operator=(SharedMemory&& other) noexcept -> SharedMemory& {
  if (this != &other) {
    close();  // Clean up current state
    data_ = other.data_;
    other.data_ = {};
  }
  return *this;
}

}  // namespace grape::realtime
