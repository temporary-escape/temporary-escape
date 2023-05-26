#pragma once

#include "../assets/assets_manager.hpp"
#include "../network/message.hpp"
#include "../scene/entity.hpp"
#include "schemas.hpp"

#include <chrono>

namespace Engine {
template <typename T> inline Network::UseFuture<T> useFuture() {
    return Network::UseFuture<T>{};
}

struct PageInfo {
    std::string start;
    std::string token;
    bool hasNext{false};

    MSGPACK_DEFINE(start, token, hasNext);
};

template <typename T> struct MessagePage {
    std::string error;
    PageInfo page;
    std::vector<T> items;

    MSGPACK_DEFINE(error, page, items);
};

struct Message {
    std::string error;
    MSGPACK_DEFINE(error);
};

// --------------------------------------------------------------------------------------------------------------------
struct MessageLoginRequest : Message {
    uint64_t secret{0};
    std::string name;
    std::string password;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), secret, name, password);
};

MESSAGE_DEFINE(MessageLoginRequest);

struct MessageLoginResponse : Message {
    std::string playerId;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), playerId);
};

MESSAGE_DEFINE(MessageLoginResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageModManifestsRequest : Message {
    MSGPACK_DEFINE(MSGPACK_BASE(Message));
};

MESSAGE_DEFINE(MessageModManifestsRequest);

struct MessageModManifestsResponse : MessagePage<ModManifest> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<ModManifest>));
};

MESSAGE_DEFINE(MessageModManifestsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePingRequest : Message {
    std::chrono::steady_clock::time_point time{};

    MSGPACK_DEFINE(MSGPACK_BASE(Message), time);
};

MESSAGE_DEFINE(MessagePingRequest);

struct MessagePingResponse : Message {
    std::chrono::steady_clock::time_point time{};

    MSGPACK_DEFINE(MSGPACK_BASE(Message), time);
};

MESSAGE_DEFINE(MessagePingResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerLocationRequest : Message {
    MSGPACK_DEFINE(MSGPACK_BASE(Message));
};

MESSAGE_DEFINE(MessagePlayerLocationRequest);

struct MessagePlayerLocationResponse : Message {
    std::optional<PlayerLocationData> location;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), location);
};

MESSAGE_DEFINE(MessagePlayerLocationResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerLocationEvent : Message {
    PlayerLocationData location;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), location);
};

MESSAGE_DEFINE(MessagePlayerLocationEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchFactionRequest : Message {
    std::string factionId;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), factionId);
};

MESSAGE_DEFINE(MessageFetchFactionRequest);

struct MessageFetchFactionResponse : Message {
    FactionData faction;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), faction);
};

MESSAGE_DEFINE(MessageFetchFactionResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchFactionsRequest : Message {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchFactionsRequest);

struct MessageFetchFactionsResponse : MessagePage<FactionData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<FactionData>));
};

MESSAGE_DEFINE(MessageFetchFactionsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchGalaxyRequest : Message {
    std::string galaxyId;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId);
};

MESSAGE_DEFINE(MessageFetchGalaxyRequest);

struct MessageFetchGalaxyResponse : Message {
    std::string name;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), name);
};

MESSAGE_DEFINE(MessageFetchGalaxyResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchRegionRequest : Message {
    std::string galaxyId;
    std::string regionId;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, regionId);
};

MESSAGE_DEFINE(MessageFetchRegionRequest);

struct MessageFetchRegionResponse : Message {
    RegionData region;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), region);
};

MESSAGE_DEFINE(MessageFetchRegionResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchRegionsRequest : Message {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchRegionsRequest);

struct MessageFetchRegionsResponse : MessagePage<RegionData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<RegionData>));
};

MESSAGE_DEFINE(MessageFetchRegionsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSectorRequest : Message {
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, systemId, sectorId);
};

MESSAGE_DEFINE(MessageFetchSectorRequest);

struct MessageFetchSectorResponse : Message {
    SectorData sector;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), sector);
};

MESSAGE_DEFINE(MessageFetchSectorResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSectorsRequest : Message {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, systemId, token);
};

MESSAGE_DEFINE(MessageFetchSectorsRequest);

struct MessageFetchSectorsResponse : MessagePage<SectorData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<SectorData>));
};

MESSAGE_DEFINE(MessageFetchSectorsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSystemRequest : Message {
    std::string galaxyId;
    std::string systemId;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, systemId);
};

MESSAGE_DEFINE(MessageFetchSystemRequest);

struct MessageFetchSystemResponse : Message {
    SystemData system;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), system);
};

MESSAGE_DEFINE(MessageFetchSystemResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSystemsRequest : Message {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchSystemsRequest);

struct MessageFetchSystemsResponse : MessagePage<SystemData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<SystemData>));
};

MESSAGE_DEFINE(MessageFetchSystemsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchPlanetsRequest : Message {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MSGPACK_DEFINE(MSGPACK_BASE(Message), galaxyId, systemId, token);
};

MESSAGE_DEFINE(MessageFetchPlanetsRequest);

struct MessageFetchPlanetsResponse : MessagePage<PlanetData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<PlanetData>));
};

MESSAGE_DEFINE(MessageFetchPlanetsResponse);

// --------------------------------------------------------------------------------------------------------------------
} // namespace Engine
