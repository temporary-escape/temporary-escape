#include "python.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
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

#define CMP "Interpreter"

using namespace Engine;
namespace py = pybind11;
using namespace py::literals;

#ifdef _WIN32
static const wchar_t pathDelimiter = ';';
#else
static const wchar_t pathDelimiter = ':';
#endif

Python::Python(const Path& home, const std::vector<Path>& paths) {
    const auto pythonHome = home.wstring() + pathDelimiter;

    std::wstring pythonPath;
    for (const auto& path : paths) {
        if (!pythonPath.empty()) {
            pythonPath += pathDelimiter;
        }

        pythonPath += path.wstring();
    }

    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
    Log::i(CMP, "Using Python home: '{}'", converter.to_bytes(pythonHome));
    Log::i(CMP, "Using Python path: '{}'", converter.to_bytes(pythonPath));


    try {
        // Initialize python
        // Py_OptimizeFlag = 1;
        Py_SetProgramName(L"TemporaryEscape");
        Py_SetPath(pythonPath.c_str());
        Py_SetPythonHome(pythonHome.c_str());
    } catch (...) {
        EXCEPTION_NESTED("Failed to set Python options");
    }

    try {
        py::scoped_interpreter guard{};

        py::exec(R"(
            import sys
            sys.dont_write_bytecode = True
        )");
    } catch (...) {
        EXCEPTION_NESTED("Failed to set Python options");
    }
}

Python::~Python() = default;
