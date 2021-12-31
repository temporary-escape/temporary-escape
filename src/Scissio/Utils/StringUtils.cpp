#include "StringUtils.hpp"

using namespace Scissio;

std::vector<std::string> Scissio::splitLast(const std::string& str, const std::string& delim) {
    const auto pos = str.find_last_of(delim);
    if (pos != std::string::npos) {
        return {
            str.substr(0, pos),
            str.substr(pos + 1),
        };
    }

    return {str};
}

std::vector<std::string> Scissio::split(const std::string& str, const std::string& delim) {
    std::vector<std::string> strings;
    size_t start = 0;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
    }
    return strings;
}

std::string Scissio::intToRomanNumeral(int value) {
    static const char* romans[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    int values[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};

    std::stringstream ss;

    for (auto int i = 0; i < 13; ++i) {
        while (value - values[i] >= 0) {
            ss << romans[i];
            value -= values[i];
        }
    }

    return ss.str();
}
