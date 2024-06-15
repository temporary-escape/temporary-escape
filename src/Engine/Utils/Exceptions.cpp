#include "Exceptions.hpp"
#include <sstream>

using namespace Engine;

void Engine::backtrace(Logger& logger, const std::exception& e) {
    logger.error(e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& ex) {
        backtrace(logger, ex);
    } catch (...) {
    }
}

static void getUserFriendlyMessage(const bool first, std::stringstream& ss, const std::exception& e) {
    auto msg = std::string{e.what()};
    auto ext = msg.find(".cpp");
    if (ext == std::string::npos) {
        ext = msg.find(".hpp");
    }
    if (ext != std::string::npos) {
        const auto pos = msg.find(' ', ext);
        if (pos != std::string::npos) {
            msg = msg.substr(pos + 1);
        } else {
            msg = msg.substr(ext + 4);
        }
    }

    if (!first) {
        ss << " ";
    }
    ss << msg;

    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& ex) {
        getUserFriendlyMessage(false, ss, ex);
    } catch (...) {
    }
}

std::string Engine::getUserFriendlyMessage(const std::exception& e) {
    std::stringstream ss;
    ::getUserFriendlyMessage(true, ss, e);
    return ss.str();
}
