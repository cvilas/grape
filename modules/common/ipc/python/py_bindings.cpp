#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "grape/ipc/config.h"
#include "grape/ipc/session.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/match.h"
#include "grape/ipc/topic.h"

namespace py = pybind11;

namespace {
// Create wrapper function that takes Config by value and moves it
void initWrapper(grape::ipc::Config config) {
    grape::ipc::init(std::move(config));
}
}

PYBIND11_MODULE(grape_ipc, module) {
    module.doc() = "Python bindings for grape::ipc";

    // Define the Config::Scope enum
    py::enum_<grape::ipc::Config::Scope>(module, "Scope")
        .value("Host", grape::ipc::Config::Scope::Host)
        .value("Network", grape::ipc::Config::Scope::Network)
        .export_values();

    // Define the Config struct
    py::class_<grape::ipc::Config>(module, "Config")
        .def(py::init<>())
        .def_readwrite("name", &grape::ipc::Config::name)
        .def_readwrite("scope", &grape::ipc::Config::scope)
        .def("__repr__",
             [](const grape::ipc::Config& config) {
                 return "Config(name='" + config.name + "', scope=" + 
                        (config.scope == grape::ipc::Config::Scope::Host ? "Host" : "Network") + ")";
             });

    // Expose the init() function via the wrapper (to pass config as rvalue )
    module.def("init", &initWrapper, "Initialize IPC session for the process", 
          py::arg("config"));
    
    // Expose the ok() function
    module.def("ok", &grape::ipc::ok, "Check if session state is nominal and error-free");

    // Bind the grape::ipc::Match::Status enum
    py::enum_<grape::ipc::Match::Status>(module, "MatchStatus")
    .value("Undefined", grape::ipc::Match::Status::Undefined)
    .value("Unmatched", grape::ipc::Match::Status::Unmatched)
    .value("Matched", grape::ipc::Match::Status::Matched)
    .export_values();

    // Bind the grape::ipc::Match struct
    py::class_<grape::ipc::Match>(module, "Match")
        .def(py::init<>())  // Default constructor
        .def_readwrite("status", &grape::ipc::Match::status);

    // Bind the grape::ipc::Topic struct
    py::class_<grape::ipc::Topic>(module, "Topic")
    .def(py::init<>())  // Default constructor
    .def_readwrite("name", &grape::ipc::Topic::name);

    // Bind the grape::ipc::Publisher class
    py::class_<grape::ipc::Publisher>(module, "Publisher")
    .def(py::init<const grape::ipc::Topic&, grape::ipc::MatchCallback&&>(),
        py::arg("topic"), py::arg("match_cb") = nullptr,
        "Create a Publisher with the specified topic and optional match callback.")
    .def("publish", [](const grape::ipc::Publisher& self, const py::bytes& data) {
        const std::string& data_str = data.cast<std::string>();
        const auto* raw_data = reinterpret_cast<const std::byte*>(data_str.data());
        const std::size_t size = data_str.size();
        const std::span<const std::byte> bytes(raw_data, size);
        self.publish(bytes);
    }, py::arg("data"), "Publish data on the topic specified at creation.");

}
