#include "python.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "bindings.hpp"
#include <Python.h>

#include <iostream>
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

using namespace Engine;
namespace py = pybind11;
using namespace py::literals;

#ifdef _WIN32
static const wchar_t pathDelimiter = ';';
#else
static const wchar_t pathDelimiter = ':';
#endif

static auto logger = createLogger(__FILENAME__);

struct Python::Module {
    py::module m;
};

Python::Python(const Path& home, const std::vector<Path>& paths) {
    const auto pythonHome = home.wstring() + pathDelimiter;

    std::wstring pythonPath = home.wstring();
    for (const auto& path : paths) {
        if (!pythonPath.empty()) {
            pythonPath += pathDelimiter;
        }

        pythonPath += path.wstring();
    }

#ifdef _WIN32
    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
#endif
    logger.info("Using Python home: '{}'", converter.to_bytes(pythonHome));
    logger.info("Using Python path: '{}'", converter.to_bytes(pythonPath));

    try {
        // Initialize python
        // Py_OptimizeFlag = 1;
        Py_SetProgramName(L"TemporaryEscape");
        Py_SetPath(pythonPath.c_str());
        Py_SetPythonHome(pythonHome.c_str());
    } catch (...) {
        EXCEPTION_NESTED("Failed to set Python options");
    }

    interpreter = std::make_unique<py::scoped_interpreter>();

    try {
        py::exec(R"(
            import sys
            sys.dont_write_bytecode = True
        )");

        py::exec(R"(
            from engine import create_logger
            logger = create_logger("builtin")
            logger.info("Hello World from Python!")
        )");
    } catch (std::exception& e) {
        EXCEPTION("Failed to set Python options error: {}", e.what());
    }
}

Python::~Python() {
    modules.clear();
}

void Engine::Python::importModule(const std::string& name) {
    try {
        modules.emplace_back(std::make_unique<Module>());
        modules.back()->m = py::module_::import(name.c_str());
    } catch (std::exception& e) {
        EXCEPTION(e.what());
    }
}

/*void Engine::Python::guard(const std::function<void()>& fn) {
    try {
        fn();
    } catch (pybind11::cast_error& e) {
        EXCEPTION(e.what());
    } catch (pybind11::error_already_set& e) {
        EXCEPTION(e.what());
    }
}*/
