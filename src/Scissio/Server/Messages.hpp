#pragma once

#include "../Network/Packet.hpp"
#include "Schemas.hpp"

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

template <typename T> struct MessageFetchRequest {
    uint64_t id = 0;
    std::string next;

    MSGPACK_DEFINE(id, next);
};

template <typename T> struct MessageFetchResponse {
    uint64_t id = 0;
    std::string next;
    std::vector<T> data;
    std::string error;

    MSGPACK_DEFINE(id, next, data, error);
};

REGISTER_MESSAGE(MessageFetchRequest<SystemData>);
REGISTER_MESSAGE(MessageFetchResponse<SystemData>);
} // namespace Scissio
