#include "Platform.hpp"
#include "Format.hpp"
#include <iostream>

using namespace Engine;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
static const auto* openUrlCommand = "start";
#elif __APPLE__
static const auto* openUrlCommand = "open";
#elif __linux__
static const auto* openUrlCommand = "xdg-open";
#else
#error "Unknown compiler"
#endif

void Engine::openWebBrowser(const std::string_view& url) {
    const auto cmd = fmt::format("{} {}", openUrlCommand, url);
    system(cmd.c_str());
}
