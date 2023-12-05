#pragma once

#include "../Library.hpp"
#include "Path.hpp"
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <spdlog/spdlog.h>
#include <utility>

namespace Engine {
using SpdLogger = spdlog::logger;

class Logger {
public:
    explicit Logger(const std::string& name, SpdLogger& root) : name{name}, log{root.clone(name)} {
    }

    void debug(const std::string& msg) {
        log->debug(msg);
    }

    void info(const std::string& msg) {
        log->info(msg);
    }

    void error(const std::string& msg) {
        log->error(msg);
    }

    void warn(const std::string& msg) {
        log->warn(msg);
    }

    template <typename... Args> void debug(const std::string& msg, Args&&... args) {
        log->debug(msg, std::forward<Args>(args)...);
    }

    template <typename... Args> void info(const std::string& msg, Args&&... args) {
        log->info(msg, std::forward<Args>(args)...);
    }

    template <typename... Args> void error(const std::string& msg, Args&&... args) {
        log->error(msg, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(const std::string& msg, Args&&... args) {
        log->warn(msg, std::forward<Args>(args)...);
    }

private:
    std::string name;
    std::shared_ptr<SpdLogger> log;
};

extern ENGINE_API Path getLoggerPath();
extern ENGINE_API Logger createLogger(const std::string_view& name);

namespace Detail {
constexpr auto logFilename(std::string_view path) {
    constexpr std::string_view sourcePath{SOURCE_ROOT};
    return path.substr(sourcePath.size() + 1).substr(0, path.find_last_of('.'));
}
} // namespace Detail
} // namespace Engine
#define LOG_FILENAME Engine::Detail::logFilename(__FILE__)
