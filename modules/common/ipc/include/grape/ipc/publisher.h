//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#pragma once

#include <memory>
#include <span>

namespace zenoh {
class Publisher;
}  // namespace zenoh

namespace grape::ipc {

class Session;

//=================================================================================================
/// Publishers post topic data. Created by Session. See also Topic.
class Publisher {
public:
  template <typename T>
  void publish(std::span<const T> bytes);

  void publish(std::span<const char> bytes);

  ~Publisher();
  Publisher(const Publisher&) = delete;
  auto operator=(const Publisher&) = delete;
  Publisher(Publisher&&) noexcept = delete;
  auto operator=(Publisher&&) noexcept = delete;

private:
  friend class Session;
  explicit Publisher(std::unique_ptr<zenoh::Publisher> zp);
  std::unique_ptr<zenoh::Publisher> impl_;
};

template <typename T>
inline void Publisher::publish(std::span<const T> bytes) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  publish({ reinterpret_cast<const char*>(bytes.data()), bytes.size_bytes() });
}

}  // namespace grape::ipc
