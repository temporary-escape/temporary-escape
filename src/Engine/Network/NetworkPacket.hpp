#pragma once

#include "../Library.hpp"
#include "../Utils/Macros.hpp"
#include <string>

namespace Engine {
enum class PacketType : uint8_t {
    None,
    Ack,
    Ping,
    Pong,
    Close,
    Data,
    DataReliable,
};

DEFINE_ENUM_FLAG_OPERATORS(PacketType)

struct PacketHeader {
    uint32_t sequence;
    PacketType type;
    char padding[3];
};

static_assert(sizeof(PacketHeader) == 8);

static constexpr size_t maxPacketSize = 1280;
} // namespace Engine
