#pragma once

#include "../Network/NetworkMessageAcceptor.hpp"
#include "../Network/Packet.hpp"
#include <array>

namespace Scissio {

class MessageServerHello {
public:
    std::string name;
    std::array<int, 3> version;

    MSGPACK_DEFINE_ARRAY(name, version);
    MESSAGE_REGISTER(MessageServerHello);
};

class MessageClientHello {
public:
    std::array<int, 3> version;

    MSGPACK_DEFINE_ARRAY(version);
    MESSAGE_REGISTER(MessageClientHello);
};

} // namespace Scissio
