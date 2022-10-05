#pragma once

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/use_future.hpp>

namespace std {
template <> struct hash<asio::ip::udp::endpoint> {
    std::size_t operator()(const asio::ip::udp::endpoint& e) const noexcept {
        return std::hash<std::string>()(e.address().to_string()) ^ hash<unsigned short>()(e.port());
    }
};

template <> struct hash<asio::ip::tcp::endpoint> {
    std::size_t operator()(const asio::ip::tcp::endpoint& e) const noexcept {
        return std::hash<std::string>()(e.address().to_string()) ^ hash<unsigned short>()(e.port());
    }
};
} // namespace std
