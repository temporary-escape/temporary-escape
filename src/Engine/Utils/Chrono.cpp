#include "Chrono.hpp"
#include <date/date.h>
#include <sstream>

using namespace Engine;

std::string Engine::timePointToIso(const std::chrono::system_clock::time_point& tp) {
    std::stringstream ss;
    ss << date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
    return ss.str();
}

std::chrono::system_clock::time_point Engine::isoToTimePoint(const std::string_view& str) {
    std::istringstream ss{std::string{str}};
    std::chrono::system_clock::time_point tp;
    ss >> date::parse("%FT%TZ", tp);
    return tp;
}

std::string Engine::timePointToLocalString(const std::chrono::system_clock::time_point& tp) {
    std::stringstream ss;

    const auto u = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
    std::time_t t = u.count();

    ss << std::put_time(std::localtime(&t), "%c");
    return ss.str();
}
