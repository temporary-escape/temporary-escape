#include "bindings.hpp"
#include "../utils/exceptions.hpp"
// clang-format off
#ifndef IS_WINDOWS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/pybind11.h>
#ifndef IS_WINDOWS
#pragma GCC diagnostic pop
#endif
// clang-format on

using namespace Engine;
namespace py = pybind11;
using namespace py::literals;

PYBIND11_EMBEDDED_MODULE(engine, m) {
    m.doc() = "game engine builtin module";

    py::class_<Logger>(m, "Logger")
        .def("info", static_cast<void (Logger::*)(const std::string&)>(&Logger::info))
        .def("debug", static_cast<void (Logger::*)(const std::string&)>(&Logger::debug))
        .def("warn", static_cast<void (Logger::*)(const std::string&)>(&Logger::warn))
        .def("error", static_cast<void (Logger::*)(const std::string&)>(&Logger::error));

    m.def("createLogger", &createLogger);
}
