#include "exceptions.hpp"

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
