#pragma once

#include "../Network/Packet.hpp"

namespace Scissio {
struct MessageLoginRequest {
    uint64_t secret;
    std::string name;
    std::string password;

    MSGPACK_DEFINE(secret, name, password);
};

REGISTER_MESSAGE(MessageLoginRequest);

struct MessageLoginResponse {
    uint64_t playerId;
    std::string error;

    MSGPACK_DEFINE(error);
};

REGISTER_MESSAGE(MessageLoginResponse);
} // namespace Scissio
