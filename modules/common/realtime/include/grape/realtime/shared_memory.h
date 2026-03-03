//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <expected>
#include <span>
#include <string>

#include "grape/error.h"

namespace grape::realtime {

//=================================================================================================
/// Creates or maps shared memory accessible across processes
///
class SharedMemory {
public:
  enum class Access : std::uint8_t { ReadOnly, ReadWrite };

  /// Checks for existence of a shared memory region.
  /// @param name Identifying name.
  /// @return true if it exists.
  [[nodiscard]] static auto exists(const std::string& name) -> bool;

  /// Removes visibility of a shared memory region.
  /// @note Until removed, a shared memory region remains persistent on the host, and can be
  /// reopened to access data contained within it.
  /// @note After remove(), new processes will not be able to open() the shared memory region, but
  /// existing processes still operating on them continue to have access until they exit or call
  /// close() on the region.
  /// @note Calling create() after remove() with the same name will create a whole new shared memory
  /// region. But beware that the previously removed shared memory region of the same name may still
  /// be in use in some processes.
  /// @param name Identifying name.
  /// @return nothing if successfully removed, else an error.
  [[nodiscard]] static auto remove(const std::string& name) -> std::expected<void, Error>;

  /// Create a shared memory region and map it to caller's process address space.
  /// @note Call data() to access mapped memory region.
  /// @note If the named shared memory region already exists, this call will fail.
  /// @param name Unique identifying name for the shared memory. The name should have a leading '/'
  /// and not contain additional '/' characters.
  /// @param size The number of bytes to allocate in the shared region.
  /// @param access Requested access rights.
  /// @return A memory segment mapped to process address space on success, else error.
  [[nodiscard]] static auto create(const std::string& name, std::size_t size, Access access)
      -> std::expected<SharedMemory, Error>;

  /// Open a pre-existing shared memory region and map it to caller's process address space.
  /// @note Call data() to access mapped memory region.
  /// @note If the named shared memory region does not exist, this call will fail.
  /// @param name Identifying name for the shared memory.
  /// @param access Requested access rights.
  /// @return A memory segment mapped to process address space on success, else error.
  [[nodiscard]] static auto open(const std::string& name, Access access)
      -> std::expected<SharedMemory, Error>;

  /// @return Raw access to shared memory segment
  /// @note Caller is responsible for safe synchronous access from multiple processes/threads.
  /// @note Caller is responsible for respecting access rights. Writing into read-only region will
  /// result in a segfault.
  [[nodiscard]] auto data() const -> std::span<std::byte>;

  /// Unmap the shared memory segment from the caller's process address space.
  /// @note To use the segment again, call open().
  /// @note Accessing shared memory region (using data()) after close() will result in a segfault.
  /// @note The OS will unmap any open shared memory automatically when the process exits. However,
  /// it is good practise to explicitly call close() when done with the region to avoid memory leaks
  /// or exhaustion.
  void close();

  ~SharedMemory();
  SharedMemory(const SharedMemory&) = delete;
  auto operator=(const SharedMemory&) -> SharedMemory& = delete;
  SharedMemory(SharedMemory&& other) noexcept;
  auto operator=(SharedMemory&& other) noexcept -> SharedMemory&;

private:
  SharedMemory() = default;  //!< call create() or open() instead
  std::span<std::byte> data_;
};

//-------------------------------------------------------------------------------------------------
inline auto SharedMemory::data() const -> std::span<std::byte> {
  return data_;
}

}  // namespace grape::realtime
