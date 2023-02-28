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

private:
    std::string name;
    const SpdLogger& root;
    SpdLogger log;
};

extern ENGINE_API Logger createLogger(std::string name);
} // namespace Engine
