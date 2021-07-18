#pragma once

#include "Log.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace Scissio {

inline void backtrace(const std::exception& e, const size_t level = 0) {
    Log::e("{} {}", std::string(level * 2, ' '), e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& ex) {
        backtrace(ex, level + 1);
    } catch (...) {
    }
}
} // namespace Scissio

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
#define STRCONCAT_NX(A, B) A##B
#define STRCONCAT(A, B) STRCONCAT_NX(A, B)

constexpr const char* pathstem(const char* path) {
    const auto* file = path;
    while (*path) {
#ifdef _WIN32
        if (*path++ == '\\') {
#else
        if (*path++ == '/') {
#endif
            file = path;
        }
    }
    return file;
}

#define __FILENAME__ pathstem(__FILE__)

#define EXCEPTION(MSG, ...)                                                                                            \
    throw std::runtime_error(fmt::format(STRCONCAT("Exception at {}:{} ", MSG), __FILENAME__, __LINE__, __VA_ARGS__));

#define EXCEPTION_NESTED(MSG, ...)                                                                                     \
    std::throw_with_nested(                                                                                            \
        std::runtime_error(fmt::format(STRCONCAT("Exception at {}:{} ", MSG), __FILENAME__, __LINE__, __VA_ARGS__)));

#define BACKTRACE(EXP, MESSAGE)                                                                                        \
    Log::e("Error at {} {}", WHERE, MESSAGE);                                                                          \
    backtrace(EXP);

#define WHERE (__FILENAME__ + std::string(":") + std::to_string(__LINE__))
