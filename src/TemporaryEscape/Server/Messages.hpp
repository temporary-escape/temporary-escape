#pragma once

#include "../Network/Packet.hpp"
#include "../Scene/Entity.hpp"
#include "Schemas.hpp"

#include <chrono>

namespace Engine {
struct MessageServerHello {
    std::string serverName;

    MSGPACK_DEFINE(serverName);
};

REGISTER_MESSAGE(MessageServerHello);

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

struct MessageStatusRequest {
    std::chrono::system_clock::time_point timePoint;

    MSGPACK_DEFINE(timePoint);
};

REGISTER_MESSAGE(MessageStatusRequest);

struct MessageStatusResponse {
    std::chrono::system_clock::time_point timePoint;

    MSGPACK_DEFINE(timePoint);
};

REGISTER_MESSAGE(MessageStatusResponse);

struct MessageSectorChanged {
    PlayerLocationData location;

    MSGPACK_DEFINE(location);
};

REGISTER_MESSAGE(MessageSectorChanged);

struct MessageEntitySync {
    std::vector<EntityPtr> entities;

    MSGPACK_DEFINE(entities);
};

REGISTER_MESSAGE(MessageEntitySync);

struct MessageEntityDeltas {
    std::vector<Entity::Delta> deltas;

    MSGPACK_DEFINE(deltas);
};

REGISTER_MESSAGE(MessageEntityDeltas);

struct MessageFetch {
    uint64_t id{0};
    std::string token;

    MSGPACK_DEFINE(id, token);
};

template <typename T> struct MessageFetchResponse : MessageFetch {
    using ItemType = T;

    ItemType data;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch), data);
};

struct MessageFetchGalaxy : MessageFetch {
    using Response = MessageFetchResponse<GalaxyData>;

    std::string galaxyId;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch), galaxyId);
};

REGISTER_MESSAGE(MessageFetchGalaxy);
REGISTER_MESSAGE(MessageFetchGalaxy::Response);

struct MessageFetchGalaxySystems : MessageFetch {
    using Response = MessageFetchResponse<std::vector<SystemData>>;

    std::string galaxyId;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch), galaxyId);
};

REGISTER_MESSAGE(MessageFetchGalaxySystems);
REGISTER_MESSAGE(MessageFetchGalaxySystems::Response);

struct MessageFetchGalaxyRegions : MessageFetch {
    using Response = MessageFetchResponse<std::vector<RegionData>>;

    std::string galaxyId;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch), galaxyId);
};

REGISTER_MESSAGE(MessageFetchGalaxyRegions);
REGISTER_MESSAGE(MessageFetchGalaxyRegions::Response);

struct MessageFetchCurrentLocation : MessageFetch {
    using Response = MessageFetchResponse<PlayerLocationData>;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch));
};

REGISTER_MESSAGE(MessageFetchCurrentLocation);
REGISTER_MESSAGE(MessageFetchCurrentLocation::Response);

struct MessageFetchSystemPlanets : MessageFetch {
    using Response = MessageFetchResponse<std::vector<SectorPlanetData>>;

    std::string galaxyId;
    std::string systemId;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch), galaxyId, systemId);
};

REGISTER_MESSAGE(MessageFetchSystemPlanets);
REGISTER_MESSAGE(MessageFetchSystemPlanets::Response);
} // namespace Engine
