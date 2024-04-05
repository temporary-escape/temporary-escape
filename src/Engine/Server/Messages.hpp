#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Network/NetworkMessage.hpp"
#include "../Scene/Entity.hpp"
#include "Schemas.hpp"

#include <chrono>

namespace Engine {
template <typename T> inline UseFuture<T> useFuture() {
    return UseFuture<T>{};
}

struct PageInfo {
    std::string start;
    std::string token;
    bool hasNext{false};

    MSGPACK_DEFINE_ARRAY(start, token, hasNext);
};

template <typename T> struct MessagePage {
    PageInfo page;
    std::vector<T> items;

    MSGPACK_DEFINE_ARRAY(page, items);
};

// --------------------------------------------------------------------------------------------------------------------
struct MessageLoginRequest {
    uint64_t secret{0};
    std::string name;
    std::string password;

    MSGPACK_DEFINE_ARRAY(secret, name, password);
};

MESSAGE_DEFINE(MessageLoginRequest);

struct MessageLoginResponse {
    std::string playerId;

    MSGPACK_DEFINE_ARRAY(playerId);
};

MESSAGE_DEFINE(MessageLoginResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageModManifestsRequest {
    bool dummy{false};

    MSGPACK_DEFINE_ARRAY(dummy);
};

MESSAGE_DEFINE(MessageModManifestsRequest);

struct MessageModManifestsResponse : MessagePage<ModManifest> {
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(MessagePage<ModManifest>));
};

MESSAGE_DEFINE(MessageModManifestsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePingRequest {
    std::chrono::steady_clock::time_point time{};

    MSGPACK_DEFINE_ARRAY(time);
};

MESSAGE_DEFINE(MessagePingRequest);

struct MessagePingResponse {
    std::chrono::steady_clock::time_point time{};

    MSGPACK_DEFINE_ARRAY(time);
};

MESSAGE_DEFINE(MessagePingResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerSpawnRequest {
    bool dummy{false};

    MSGPACK_DEFINE_ARRAY(dummy);
};

MESSAGE_DEFINE(MessagePlayerSpawnRequest);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerLocationEvent {
    PlayerLocationData location;
    SectorData sector;
    SystemData system;

    MSGPACK_DEFINE_ARRAY(location, sector, system);
};

MESSAGE_DEFINE(MessagePlayerLocationEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageSceneUpdateEvent {
    // void
};

MESSAGE_DEFINE(MessageSceneUpdateEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerControlEvent {
    EntityId entityId{0};

    MSGPACK_DEFINE_ARRAY(entityId);
};

MESSAGE_DEFINE(MessagePlayerControlEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageActionApproach {
    EntityId entityId{0};

    MSGPACK_DEFINE_ARRAY(entityId);
};

MESSAGE_DEFINE(MessageActionApproach);

// --------------------------------------------------------------------------------------------------------------------
struct MessageActionOrbit {
    EntityId entityId{0};
    float radius{0};

    MSGPACK_DEFINE_ARRAY(entityId, radius);
};

MESSAGE_DEFINE(MessageActionOrbit);

// --------------------------------------------------------------------------------------------------------------------
struct MessageActionKeepDistance {
    EntityId entityId{0};
    float distance{0};

    MSGPACK_DEFINE_ARRAY(entityId, distance);
};

MESSAGE_DEFINE(MessageActionKeepDistance);

// --------------------------------------------------------------------------------------------------------------------
struct MessageActionStopMovement {
    bool dummy{false};

    MSGPACK_DEFINE_ARRAY(dummy);
};

MESSAGE_DEFINE(MessageActionStopMovement);

// --------------------------------------------------------------------------------------------------------------------
struct MessageActionGoDirection {
    Vector3 direction;

    MSGPACK_DEFINE_ARRAY(direction);
};

MESSAGE_DEFINE(MessageActionGoDirection);

// --------------------------------------------------------------------------------------------------------------------
struct MessageActionWarpTo {
    std::string sectorId;

    MSGPACK_DEFINE_ARRAY(sectorId);
};

MESSAGE_DEFINE(MessageActionWarpTo);

// --------------------------------------------------------------------------------------------------------------------
struct MessageControlTargetEvent {
    uint64_t entityId{0};

    MSGPACK_DEFINE_ARRAY(entityId);
};

MESSAGE_DEFINE(MessageControlTargetEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageShipStatusEvent {
    EntityId approaching;
    float orbitRadius;
    float keepAtDistance;

    MSGPACK_DEFINE_ARRAY(approaching, orbitRadius, keepAtDistance);
};

MESSAGE_DEFINE(MessageShipStatusEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageSceneBulletsEvent {
    // void
};

MESSAGE_DEFINE(MessageSceneBulletsEvent);
} // namespace Engine
