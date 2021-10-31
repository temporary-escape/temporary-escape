#include "Log.hpp"
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace Scissio;

static std::unique_ptr<spdlog::logger> logger;

void Log::configure(bool debug) {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] %v");

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("scissio.log", true);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] %v");

    ::logger.reset(new spdlog::logger("Scissio", {consoleSink, fileSink})); // NOLINT(modernize-make-unique)
    ::logger->flush_on(spdlog::level::debug);
    ::logger->set_level(spdlog::level::debug);
}

void Log::i(const std::string& cmp, const std::string& msg) {
    ::logger->info("[{}] {}", cmp, msg);
}

void Log::w(const std::string& cmp, const std::string& msg) {
    ::logger->warn("[{}] {}", cmp, msg);
}

void Log::e(const std::string& cmp, const std::string& msg) {
    ::logger->error("[{}] {}", cmp, msg);
}

void Log::d(const std::string& cmp, const std::string& msg) {
    ::logger->debug("[{}] {}", cmp, msg);
}
