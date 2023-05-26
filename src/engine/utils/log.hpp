#pragma once

#include "../library.hpp"
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <spdlog/spdlog.h>
#include <utility>

namespace Engine {
using SpdLogger = std::shared_ptr<spdlog::logger>;

class Logger {
public:
    explicit Logger(std::string name, const SpdLogger& root) : name{std::move(name)}, root{root} {
    }

    void debug(const std::string& msg) {
        getLogger()->debug(msg);
    }

    void info(const std::string& msg) {
        getLogger()->info(msg);
    }

    void error(const std::string& msg) {
        getLogger()->error(msg);
    }

    void warn(const std::string& msg) {
        getLogger()->warn(msg);
    }

    template <typename... Args> void debug(const std::string& msg, Args&&... args) {
        getLogger()->debug(msg, std::forward<Args>(args)...);
    }

    template <typename... Args> void info(const std::string& msg, Args&&... args) {
        getLogger()->info(msg, std::forward<Args>(args)...);
    }

    template <typename... Args> void error(const std::string& msg, Args&&... args) {
        getLogger()->error(msg, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(const std::string& msg, Args&&... args) {
        getLogger()->warn(msg, std::forward<Args>(args)...);
    }

    SpdLogger& getLogger() {
        if (!log) {
            log = root->clone(name);
        }
        return log;
    }

    static void bind(Lua& lua);

private:
    std::string name;
    const SpdLogger& root;
    SpdLogger log;
};

extern ENGINE_API Logger createLogger(const std::string_view& name);

namespace Detail {
constexpr auto logFilename(std::string_view path) {
    constexpr std::string_view sourcePath{SOURCE_ROOT};
    return path.substr(sourcePath.size() + 1).substr(0, path.find_last_of('.'));
}
} // namespace Detail
} // namespace Engine
#define LOG_FILENAME Engine::Detail::logFilename(__FILE__)
