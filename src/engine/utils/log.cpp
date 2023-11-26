#include "log.hpp"
#include "../server/lua.hpp"
#include "path.hpp"
#include <memory>
#include <sol/sol.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace Engine;

static std::unique_ptr<spdlog::logger> createLogger(const Path& path) {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] [%n] %v");

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] [%n] %v");

    std::unique_ptr<spdlog::logger> logger{
        new spdlog::logger("Engine", {consoleSink, fileSink}),
    }; // NOLINT(modernize-make-unique)
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    return logger;
}

Path Engine::getLoggerPath() {
    return getAppDataPath() / "TemporaryEscape.log";
}

Logger Engine::createLogger(const std::string_view& name) {
    static auto root = ::createLogger(getLoggerPath());
    return Logger{std::string{name}, *root};
}

void Logger::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Logger>("Logger");
    cls["info"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::info);
    cls["debug"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::debug);
    cls["warn"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::warn);
    cls["error"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::error);
}
