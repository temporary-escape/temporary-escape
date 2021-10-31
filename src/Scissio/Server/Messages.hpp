#pragma once

#include "../Network/Packet.hpp"

namespace Scissio {
struct MessageLoginRequest {
    uint64_t uid;
    std::string name;
    std::string password;

    MSGPACK_DEFINE(uid, name, password);
};

REGISTER_MESSAGE(MessageLoginRequest);

struct MessageLoginResponse {
    std::string error;

    MSGPACK_DEFINE(error);
};

REGISTER_MESSAGE(MessageLoginResponse);
} // namespace Scissio
