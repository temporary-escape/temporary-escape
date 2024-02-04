#pragma once

#include "../Config.hpp"
#include "../Utils/Log.hpp"
#include <asio.hpp>
#include <asio/ssl.hpp>

namespace Engine {
struct UrlParts {
    std::string proto;
    std::string host;
    uint16_t port;
    std::string path;
};

ENGINE_API std::optional<UrlParts> parseUrl(const std::string_view& url);
} // namespace Engine

template <> struct fmt::formatter<asio::ip::tcp::endpoint> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(asio::ip::tcp::endpoint const& endpoint, FormatContext& ctx) {
        if (endpoint.address().is_v6()) {
            return fmt::format_to(ctx.out(), "[{}]:{}", endpoint.address().to_string(), endpoint.port());
        } else {
            return fmt::format_to(ctx.out(), "{}:{}", endpoint.address().to_string(), endpoint.port());
        }
    }
};

template <> struct fmt::formatter<asio::ip::udp::endpoint> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(asio::ip::udp::endpoint const& endpoint, FormatContext& ctx) {
        if (endpoint.address().is_v6()) {
            return fmt::format_to(ctx.out(), "[{}]:{}", endpoint.address().to_string(), endpoint.port());
        } else {
            return fmt::format_to(ctx.out(), "{}:{}", endpoint.address().to_string(), endpoint.port());
        }
    }
};
