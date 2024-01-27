#include "Chrono.hpp"
#include <date/date.h>
#include <sstream>

using namespace Engine;

std::string Engine::timePointToIso(const std::chrono::system_clock::time_point& tp) {
    std::stringstream ss;
    ss << date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
    return ss.str();
}

std::chrono::system_clock::time_point Engine::isoToTimePoint(const std::string& str) {
    std::istringstream ss{str};
    std::chrono::system_clock::time_point tp;
    ss >> date::parse("%FT%TZ", tp);
    return tp;
}
