#include "StringUtils.hpp"

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
    size_t start = 0;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.emplace_back(str.substr(start, end - start));
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
