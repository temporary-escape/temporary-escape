#include "StringUtils.hpp"
#include <algorithm>

using namespace Engine;

std::vector<std::string> Engine::splitLast(const std::string_view& str, const std::string_view& delim) {
    const auto pos = str.find_last_of(delim);
    if (pos != std::string::npos) {
        return {
            std::string{str.substr(0, pos)},
            std::string{str.substr(pos + 1)},
        };
    }

    return {std::string{str}};
}

std::vector<std::string> Engine::split(const std::string_view& str, const std::string_view& delim) {
    std::vector<std::string> strings;
    size_t pos = 0;
    size_t next = 0;
    while ((next = str.find(delim, pos)) != std::string::npos) {
        strings.emplace_back(str.substr(pos, next - pos));
        pos = next + delim.size();
    }

    if (pos < str.size()) {
        strings.emplace_back(str.substr(pos));
    }

    return strings;
}

std::string Engine::intToRomanNumeral(int value) {
    static const char* romans[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    int values[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};

    std::stringstream ss;

    for (int i = 0; i < 13; ++i) {
        while (value - values[i] >= 0) {
            ss << romans[i];
            value -= values[i];
        }
    }

    return ss.str();
}

std::string Engine::toLower(const std::string_view& str) {
    std::string res{str};
    std::transform(res.begin(), res.end(), res.begin(), [](char c) { return std::tolower(c); });
    return res;
}

std::string Engine::toHexString(const void* src, const size_t size) {
    static const std::string_view chars = "0123456789abcdef";

    std::string res;
    res.resize(size * 2);

    const auto* ptr = reinterpret_cast<const char*>(src);
    for (size_t i = 0; i < size; i++) {
        res[i * 2 + 0] = chars[(ptr[i] & 0xF0) >> 4];
        res[i * 2 + 1] = chars[(ptr[i] & 0x0F) >> 0];
    }

    return res;
}

bool Engine::endsWith(const std::string_view& str, const std::string_view& ending) {
    if (str.length() >= ending.length()) {
        return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool Engine::startsWith(const std::string_view& str, const std::string_view& start) {
    if (str.length() >= start.length()) {
        return (0 == str.compare(0, start.length(), start));
    } else {
        return false;
    }
}
