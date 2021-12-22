#pragma once

#include "../Network/Packet.hpp"
#include "Schemas.hpp"

#include <chrono>

namespace Scissio {
struct MessageLoginRequest {
    uint64_t secret;
    std::string name;
    std::string password;

    MSGPACK_DEFINE(secret, name, password);
};

REGISTER_MESSAGE(MessageLoginRequest);

struct MessageLoginResponse {
    std::string playerId;
    std::string error;

    MSGPACK_DEFINE(error);
};

REGISTER_MESSAGE(MessageLoginResponse);

struct MessagePingRequest {
    std::chrono::system_clock::time_point timePoint;

    MSGPACK_DEFINE(timePoint);
};

REGISTER_MESSAGE(MessagePingRequest);

struct MessagePingResponse {
    std::chrono::system_clock::time_point timePoint;

    MSGPACK_DEFINE(timePoint);
};

REGISTER_MESSAGE(MessagePingResponse);

struct MessageLatencyRequest {
    std::chrono::system_clock::time_point timePoint;

    MSGPACK_DEFINE(timePoint);
};

REGISTER_MESSAGE(MessageLatencyRequest);

struct MessageLatencyResponse {
    std::chrono::system_clock::time_point timePoint;

    MSGPACK_DEFINE(timePoint);
};

REGISTER_MESSAGE(MessageLatencyResponse);

template <typename T> struct MessageFetchRequest {
    uint64_t id = 0;
    std::string prefix;
    std::string start;

    MSGPACK_DEFINE(id, prefix, start);
};

template <typename T> struct MessageFetchResponse {
    uint64_t id = 0;
    std::string prefix;
    std::string next;
    std::vector<T> data;
    std::string error;

    MSGPACK_DEFINE(id, prefix, next, data, error);
};

REGISTER_MESSAGE(MessageFetchRequest<SystemData>);
REGISTER_MESSAGE(MessageFetchResponse<SystemData>);
} // namespace Scissio
