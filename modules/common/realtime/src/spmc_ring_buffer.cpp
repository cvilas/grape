//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/spmc_ring_buffer.h"

#include <atomic>
#include <format>

#include "grape/exception.h"
#include "grape/shared_memory.h"

namespace {
//-------------------------------------------------------------------------------------------------
auto shmName(std::string_view name) -> std::string {
  return std::format("/{}", name);
}
}  // namespace

namespace grape::spmc_ring_buffer {

//=================================================================================================
struct Control {
  static constexpr auto MAGIC = 0x00465542434D5053U;  //!< "SPMCBUF\0";
  std::uint64_t magic{};
  std::uint64_t write_count{};
  Config config;
};

//=================================================================================================
class Writer::Impl : public SharedMemory {
public:
  Impl(std::string name, SharedMemory shm)
    : SharedMemory(std::move(shm)), shm_name_(std::move(name)) {
  }

  auto control() -> Control& {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return *reinterpret_cast<Control*>(SharedMemory::data().data());
  }

  void visit(const WriterFunc& func) {
    auto& ctrl = control();
    static const auto ctrl_size = sizeof(Control);
    const auto frame_len = ctrl.config.frame_length;
    const auto frame_start = ctrl_size + ((ctrl.write_count % ctrl.config.num_frames) * frame_len);

    func(data().subspan(frame_start, frame_len));
    ctrl.write_count++;
  }

  void cleanup() {
    SharedMemory::close();
    std::ignore = SharedMemory::remove(shm_name_);
  }

private:
  std::string shm_name_;
};

//-------------------------------------------------------------------------------------------------
auto Writer::create(std::string_view name, const Config& config) -> std::expected<Writer, Error> {
  const auto ctrl_size = sizeof(Control);
  const auto data_size = config.frame_length * config.num_frames;
  if (data_size == 0) {
    return std::unexpected{ Error{
        "Invalid configuration. Frame length or num frames cannot be zero" } };
  }
  const auto sz = ctrl_size + data_size;
  const auto shm_name = shmName(name);
  auto maybe_shm = SharedMemory::create(shm_name, sz, SharedMemory::Access::ReadWrite);
  if (not maybe_shm) {
    return std::unexpected{ maybe_shm.error() };
  }
  auto impl = std::make_unique<Writer::Impl>(shm_name, std::move(maybe_shm.value()));
  auto& control = impl->control();
  control.config = config;
  control.write_count = 0U;
  control.magic = Control::MAGIC;  // Marks as initialised
  return Writer{ std::move(impl) };
  // TODO:
  //  - resolve race with reader in accessing control region before writer has initialised it
  //  - make write_count atomic. Is it even possible when mapped in shared memory?
  //  - make num_frames power of 2, avoid cost of %
  //  - move contents of this function into Writer::Impl because it is already derived from shm
}

//-------------------------------------------------------------------------------------------------
Writer::~Writer() {
  if (impl_ != nullptr) {
    impl_->cleanup();
  }
};

//-------------------------------------------------------------------------------------------------
Writer::Writer(std::unique_ptr<Impl> impl) : impl_{ std::move(impl) } {
}

//-------------------------------------------------------------------------------------------------
void Writer::visit(const WriterFunc& func) {
  if (impl_ and func) {
    impl_->visit(func);
  }
}
}  // namespace grape::spmc_ring_buffer