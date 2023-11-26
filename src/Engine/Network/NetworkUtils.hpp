#include "../Utils/Log.hpp"
#include <asio.hpp>
#include <asio/ssl.hpp>

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
