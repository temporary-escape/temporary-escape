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
