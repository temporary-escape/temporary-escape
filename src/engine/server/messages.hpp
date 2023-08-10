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

    MSGPACK_DEFINE(start, token, hasNext);
};

template <typename T> struct MessagePage {
    PageInfo page;
    std::vector<T> items;

    MSGPACK_DEFINE(page, items);
};

// --------------------------------------------------------------------------------------------------------------------
struct MessageLoginRequest {
    uint64_t secret{0};
    std::string name;
    std::string password;

    MSGPACK_DEFINE(secret, name, password);
};

MESSAGE_DEFINE(MessageLoginRequest);

struct MessageLoginResponse {
    std::string playerId;

    MSGPACK_DEFINE(playerId);
};

MESSAGE_DEFINE(MessageLoginResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageModManifestsRequest {
    bool dummy{false};

    MSGPACK_DEFINE(dummy);
};

MESSAGE_DEFINE(MessageModManifestsRequest);

struct MessageModManifestsResponse : MessagePage<ModManifest> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<ModManifest>));
};

MESSAGE_DEFINE(MessageModManifestsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePingRequest {
    std::chrono::steady_clock::time_point time{};

    MSGPACK_DEFINE(time);
};

MESSAGE_DEFINE(MessagePingRequest);

struct MessagePingResponse {
    std::chrono::steady_clock::time_point time{};

    MSGPACK_DEFINE(time);
};

MESSAGE_DEFINE(MessagePingResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerLocationRequest {
    bool dummy{false};

    MSGPACK_DEFINE(dummy);
};

MESSAGE_DEFINE(MessagePlayerLocationRequest);

struct MessagePlayerLocationResponse {
    std::optional<PlayerLocationData> location;

    MSGPACK_DEFINE(location);
};

MESSAGE_DEFINE(MessagePlayerLocationResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessagePlayerLocationEvent {
    PlayerLocationData location;
    SectorData sector;

    MSGPACK_DEFINE(location, sector);
};

MESSAGE_DEFINE(MessagePlayerLocationEvent);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchFactionRequest {
    std::string factionId;

    MSGPACK_DEFINE(factionId);
};

MESSAGE_DEFINE(MessageFetchFactionRequest);

struct MessageFetchFactionResponse {
    FactionData faction;

    MSGPACK_DEFINE(faction);
};

MESSAGE_DEFINE(MessageFetchFactionResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchFactionsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchFactionsRequest);

struct MessageFetchFactionsResponse : MessagePage<FactionData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<FactionData>));
};

MESSAGE_DEFINE(MessageFetchFactionsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchGalaxyRequest {
    std::string galaxyId;

    MSGPACK_DEFINE(galaxyId);
};

MESSAGE_DEFINE(MessageFetchGalaxyRequest);

struct MessageFetchGalaxyResponse {
    std::string name;

    MSGPACK_DEFINE(name);
};

MESSAGE_DEFINE(MessageFetchGalaxyResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchRegionRequest {
    std::string galaxyId;
    std::string regionId;

    MSGPACK_DEFINE(galaxyId, regionId);
};

MESSAGE_DEFINE(MessageFetchRegionRequest);

struct MessageFetchRegionResponse {
    RegionData region;

    MSGPACK_DEFINE(region);
};

MESSAGE_DEFINE(MessageFetchRegionResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchRegionsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchRegionsRequest);

struct MessageFetchRegionsResponse : MessagePage<RegionData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<RegionData>));
};

MESSAGE_DEFINE(MessageFetchRegionsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSectorRequest {
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    MSGPACK_DEFINE(galaxyId, systemId, sectorId);
};

MESSAGE_DEFINE(MessageFetchSectorRequest);

struct MessageFetchSectorResponse {
    SectorData sector;

    MSGPACK_DEFINE(sector);
};

MESSAGE_DEFINE(MessageFetchSectorResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSectorsRequest {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, systemId, token);
};

MESSAGE_DEFINE(MessageFetchSectorsRequest);

struct MessageFetchSectorsResponse : MessagePage<SectorData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<SectorData>));
};

MESSAGE_DEFINE(MessageFetchSectorsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSystemRequest {
    std::string galaxyId;
    std::string systemId;

    MSGPACK_DEFINE(galaxyId, systemId);
};

MESSAGE_DEFINE(MessageFetchSystemRequest);

struct MessageFetchSystemResponse {
    SystemData system;

    MSGPACK_DEFINE(system);
};

MESSAGE_DEFINE(MessageFetchSystemResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchSystemsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchSystemsRequest);

struct MessageFetchSystemsResponse : MessagePage<SystemData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<SystemData>));
};

MESSAGE_DEFINE(MessageFetchSystemsResponse);

// --------------------------------------------------------------------------------------------------------------------
struct MessageFetchPlanetsRequest {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, systemId, token);
};

MESSAGE_DEFINE(MessageFetchPlanetsRequest);

struct MessageFetchPlanetsResponse : MessagePage<PlanetData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<PlanetData>));
};

MESSAGE_DEFINE(MessageFetchPlanetsResponse);

// --------------------------------------------------------------------------------------------------------------------
} // namespace Engine
