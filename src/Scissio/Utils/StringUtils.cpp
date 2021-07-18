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
