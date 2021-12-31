#pragma once

#include "../Network/Packet.hpp"
#include "../Scene/Entity.hpp"
#include "Schemas.hpp"

#include <chrono>

namespace Scissio {
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
    std::string compoundId;

    MSGPACK_DEFINE(compoundId);
};

REGISTER_MESSAGE(MessageSectorChanged);

struct MessageEntitySync {
    std::vector<EntityPtr> entities;

    MSGPACK_DEFINE(entities);
};

REGISTER_MESSAGE(MessageEntitySync);

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

struct MessageFetchGalaxySystems : MessageFetch {
    using Response = MessageFetchResponse<std::vector<SystemData>>;

    std::string galaxyId;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch), galaxyId);
};

REGISTER_MESSAGE(MessageFetchGalaxySystems);
REGISTER_MESSAGE(MessageFetchGalaxySystems::Response);

struct MessageFetchCurrentLocation : MessageFetch {
    using Response = MessageFetchResponse<PlayerLocationData>;

    MSGPACK_DEFINE(MSGPACK_BASE(MessageFetch));
};

REGISTER_MESSAGE(MessageFetchCurrentLocation);
REGISTER_MESSAGE(MessageFetchCurrentLocation::Response);

/*template <typename T> struct MessageFetchRequest {
    uint64_t id = 0;
    std::string prefix;
    std::string start;

    MSGPACK_DEFINE(id, prefix, start);
};

template <typename T> struct MessageFetchResponse {
    uint64_t id = 0;
    std::string prefix;
    std::string next;
    std::vector<T> data;
    std::string error;

    MSGPACK_DEFINE(id, prefix, next, data, error);
};

REGISTER_MESSAGE(MessageFetchRequest<SystemData>);
REGISTER_MESSAGE(MessageFetchResponse<SystemData>);*/
} // namespace Scissio
