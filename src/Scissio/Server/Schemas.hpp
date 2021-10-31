#pragma once

#include "../Utils/Msgpack.hpp"

namespace Scissio {
struct PlayerData {
    uint64_t uid;
    std::string name;
    bool admin;

    MSGPACK_DEFINE(uid, name, admin);
};
} // namespace Scissio
