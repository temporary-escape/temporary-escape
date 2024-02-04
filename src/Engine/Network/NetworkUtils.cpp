#include "NetworkUtils.hpp"
#include <regex>

using namespace Engine;

static const std::regex urlPattern{R"(^(http[s]?|ws[s]?)\:\/\/([\w.-]+)(\:[0-9]+)?([\/\w.-]+)?$)"};

std::optional<UrlParts> Engine::parseUrl(const std::string_view& url) {
    std::match_results<std::string_view::const_iterator> m;
    if (!std::regex_match(url.begin(), url.end(), m, urlPattern)) {
        return std::nullopt;
    }

    try {
        UrlParts parts{};

        parts.proto = m[1];
        parts.host = m[2];
        if (m[3].matched) {
            parts.port = std::stoi(m[3].str().substr(1));
        } else if (parts.proto == "http" || parts.proto == "ws") {
            parts.port = 80;
        } else if (parts.proto == "https" || parts.proto == "wss") {
            parts.port = 443;
        }
        if (m[4].matched) {
            parts.path = m[4];
        } else {
            parts.path = "/";
        }

        return parts;
    } catch (std::exception& e) {
        return std::nullopt;
    }
}
