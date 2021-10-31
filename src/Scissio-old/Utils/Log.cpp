#define NOMINMAX 1
#include <ctime>
#include <mutex>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "Log.hpp"
#include <fmt/chrono.h>
#include <iostream>
#include <termcolor/termcolor.hpp>

static std::mutex mut;

using namespace Scissio;

static const char* typeToStr(const Log::Type type) {
    switch (type) {
    case Log::Type::E: {
        return "ERROR  ";
    }
    case Log::Type::W: {
        return "WARN   ";
    }
    case Log::Type::V: {
        return "VERBOSE";
    }
    case Log::Type::D: {
        return "DEBUG  ";
    }
    case Log::Type::I: {
        return "INFO   ";
    }
    default: {
        return "       ";
    }
    }
}

static void typeToColor(const Log::Type type) {
    switch (type) {
    case Log::Type::E: {
        std::cout << termcolor::red;
        break;
    }
    case Log::Type::W: {
        std::cout << termcolor::yellow;
        break;
    }
    case Log::Type::V: {
        std::cout << termcolor::white;
        break;
    }
    case Log::Type::D: {
        std::cout << termcolor::white;
        break;
    }
    case Log::Type::I: {
        std::cout << termcolor::cyan;
        break;
    }
    default: {
        break;
    }
    }
}

void Log::print(Type type, const std::string& str) {
    std::lock_guard<std::mutex> lock(mut);

    time_t t = time(nullptr);
    struct tm* now = localtime(&t);

    typeToColor(type);
    std::cout << fmt::format("{:%Y-%m-%d %H:%M:%S} | {} | {}", *now, typeToStr(type), str) << std::endl;
    std::cout << termcolor::reset;
}

void Log::e(const std::string& str) {
    print(Type::E, str);
}

void Log::w(const std::string& str) {
    print(Type::W, str);
}

void Log::d(const std::string& str) {
    print(Type::D, str);
}

void Log::v(const std::string& str) {
    print(Type::V, str);
}

void Log::i(const std::string& str) {
    print(Type::I, str);
}
