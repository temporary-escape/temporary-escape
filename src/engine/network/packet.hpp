#pragma once

#include "../library.hpp"
#include <iostream>
#include <msgpack.hpp>

namespace Engine::Network {
struct ENGINE_API PacketInfo {
    uint64_t id{0};
    uint64_t reqId{0};
    bool isResponse{false};

    MSGPACK_DEFINE_ARRAY(id, reqId, isResponse);
};
} // namespace Engine::Network
