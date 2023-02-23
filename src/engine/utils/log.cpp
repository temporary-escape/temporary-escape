#include "log.hpp"
#include "path.hpp"
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace Engine;

static std::unique_ptr<spdlog::logger> createLogger(const std::optional<Path>& path) {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] %v");

    const std::string filename = path.has_value() ? path.value().string() : "TemporaryEscape.log";

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] %v");

    std::unique_ptr<spdlog::logger> logger{
        new spdlog::logger("Engine", {consoleSink, fileSink}),
    }; // NOLINT(modernize-make-unique)
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::debug);

    return logger;
}

static std::unique_ptr<spdlog::logger> logger = createLogger(getAppDataPath() / "TemporaryEscape.log");

void Engine::Log::i(const std::string& cmp, const std::string& msg) {
    ::logger->info("[{}] {}", cmp, msg);
}

void Engine::Log::w(const std::string& cmp, const std::string& msg) {
    ::logger->warn("[{}] {}", cmp, msg);
}

void Engine::Log::e(const std::string& cmp, const std::string& msg) {
    ::logger->error("[{}] {}", cmp, msg);
}

void Engine::Log::d(const std::string& cmp, const std::string& msg) {
    ::logger->debug("[{}] {}", cmp, msg);
}
