#pragma once

#include "../assets/registry.hpp"
#include "../network/message.hpp"
#include "../scene/entity.hpp"
#include "world.hpp"

#include <chrono>

namespace Engine {
struct MessageLoginRequest {
    uint64_t secret{0};
    std::string name;
    std::string password;

    MESSAGE_DEFINE(MessageLoginRequest, secret, name, password);
};

struct MessageLoginResponse {
    std::string error;
    std::string playerId;

    MESSAGE_DEFINE(MessageLoginResponse, error, playerId);
};

struct MessageModsInfoRequest {
    bool dummy{false};

    MESSAGE_DEFINE(MessageModsInfoRequest, dummy);
};

struct MessageModsInfoResponse {
    std::vector<ModManifest> manifests;

    MESSAGE_DEFINE(MessageModsInfoResponse, manifests);
};

struct MessagePingRequest {
    std::chrono::steady_clock::time_point time{};

    MESSAGE_DEFINE(MessagePingRequest, time);
};

struct MessagePingResponse {
    std::chrono::steady_clock::time_point time{};

    MESSAGE_DEFINE(MessagePingResponse, time);
};

struct MessageSpawnRequest {
    bool dummy{false};

    MESSAGE_DEFINE(MessageSpawnRequest, dummy);
};

struct MessageSpawnResponse {
    PlayerLocationData location;

    MESSAGE_DEFINE(MessageSpawnResponse, location);
};

struct MessagePlayerLocationChanged {
    PlayerLocationData location;

    MESSAGE_DEFINE(MessagePlayerLocationChanged, location);
};

struct MessageSceneEntitiesChanged {
    std::vector<EntityProxyPtr> entities;

    MESSAGE_DEFINE(MessageSceneEntitiesChanged, entities);
};

struct MessageSceneDeltasChanged {
    std::vector<Entity::Delta> deltas;

    MESSAGE_DEFINE(MessageSceneDeltasChanged, deltas);
};

struct MessageShipMovementRequest {
    bool left{false};
    bool right{false};
    bool up{false};
    bool down{false};

    MESSAGE_DEFINE(MessageShipMovementRequest, left, right, up, down);
};

struct MessageShipMovementResponse {
    bool moving{false};

    MESSAGE_DEFINE(MessageShipMovementResponse, moving);
};
} // namespace Engine
