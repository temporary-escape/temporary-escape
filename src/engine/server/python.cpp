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
static const auto pathDelimiter = ';';
#else
static const auto pathDelimiter = ':';
#endif

Python::Python(const Path& home, const std::vector<Path>& paths) {
    const auto pythonHome = fmt::format("{}{}", home.string(), pathDelimiter);

    std::string pythonPath;
    for (const auto& path : paths) {
        if (!pythonPath.empty()) {
            pythonPath += pathDelimiter;
        }

        pythonPath += path.string();
    }

    Log::i(CMP, "Using Python home: '{}'", pythonHome);
    Log::i(CMP, "Using Python path: '{}'", pythonPath);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring pythonHomeW = converter.from_bytes(pythonHome);
    std::wstring pythonPathW = converter.from_bytes(pythonPath);

    try {
        // Initialize python
        Py_OptimizeFlag = 1;
        Py_SetProgramName(L"TemporaryEscape");
        Py_SetPath(pythonPathW.c_str());
        Py_SetPythonHome(pythonHomeW.c_str());
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
