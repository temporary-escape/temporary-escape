#pragma once

#include "../assets/assets_manager.hpp"
#include "../network/network_message.hpp"
#include "../scene/entity.hpp"
#include "schemas.hpp"

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

    MSGPACK_DEFINE_ARRAY(location, sector);
};

MESSAGE_DEFINE(MessagePlayerLocationEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageSceneUpdateEvent {
    // void
};

MESSAGE_DEFINE(MessageSceneUpdateEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerControlEvent {
    uint64_t entityId{0};

    MSGPACK_DEFINE_ARRAY(entityId);
};

MESSAGE_DEFINE(MessagePlayerControlEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageControlMovementEvent {
    float speed{0.0f};
    int leftRight{0};
    int upDown{0};
    bool boost{false};

    MSGPACK_DEFINE_ARRAY(speed, leftRight, upDown, boost);
};

MESSAGE_DEFINE(MessageControlMovementEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageControlTargetEvent {
    uint64_t entityId{0};

    MSGPACK_DEFINE_ARRAY(entityId);
};

MESSAGE_DEFINE(MessageControlTargetEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageSceneBulletsEvent {
    // void
};

MESSAGE_DEFINE(MessageSceneBulletsEvent);
} // namespace Engine
