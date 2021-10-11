#pragma once

#include "../Network/Packet.hpp"
#include "../Scene/Entity.hpp"
#include "World.hpp"

#include <array>
#include <type_traits>

namespace Scissio {
struct MessageServerError {
    std::string msg;

    MSGPACK_DEFINE_ARRAY(msg);
};

REGISTER_MESSAGE(MessageServerError);

struct MessageHelloRequest {
    std::array<int, 3> version;

    MSGPACK_DEFINE_ARRAY(version);
};

REGISTER_MESSAGE(MessageHelloRequest);

struct MessageHelloResponse {
    std::string name;
    std::array<int, 3> version;

    MSGPACK_DEFINE_ARRAY(name, version);
};

REGISTER_MESSAGE(MessageHelloResponse);

struct MessageLoginRequest {
    uint64_t uid{0};
    std::string name;
    std::string password;

    MSGPACK_DEFINE(uid, name, password);
};

REGISTER_MESSAGE(MessageLoginRequest);

struct MessageLoginResponse {
    std::string error;

    MSGPACK_DEFINE(error);
};

REGISTER_MESSAGE(MessageLoginResponse);

template <typename T> struct MessageResourceRequest {
    size_t cont{0};

    MSGPACK_DEFINE_ARRAY(cont);
};

template <typename T> using MessageResourceResponse = Page<T>;

using MessageSystemsRequest = MessageResourceRequest<SystemDto>;
using MessageSystemsResponse = MessageResourceResponse<SystemDto>;

REGISTER_MESSAGE(MessageSystemsRequest);
REGISTER_MESSAGE(MessageSystemsResponse);

using MessageRegionsRequest = MessageResourceRequest<RegionDto>;
using MessageRegionsResponse = MessageResourceResponse<RegionDto>;

REGISTER_MESSAGE(MessageRegionsRequest);
REGISTER_MESSAGE(MessageRegionsResponse);

using MessageBlocksRequest = MessageResourceRequest<BlockDto>;
using MessageBlocksResponse = MessageResourceResponse<BlockDto>;

REGISTER_MESSAGE(MessageBlocksRequest);
REGISTER_MESSAGE(MessageBlocksResponse);

struct MessageSectorChanged {
    uint64_t galaxyId{0};
    uint64_t systemId{0};
    SectorDto sector;

    MSGPACK_DEFINE_ARRAY(galaxyId, systemId, sector);
};

REGISTER_MESSAGE(MessageSectorChanged);

struct MessageSectorStatusRequest {
    MSGPACK_DEFINE_ARRAY();
};

REGISTER_MESSAGE(MessageSectorStatusRequest);

struct MessageSectorStatusResponse {
    bool loaded{false};

    MSGPACK_DEFINE_ARRAY(loaded);
};

REGISTER_MESSAGE(MessageSectorStatusResponse);

struct MessageEntityBatch {
    std::vector<EntityPtr> entities;

    MSGPACK_DEFINE_ARRAY(entities);
};

REGISTER_MESSAGE(MessageEntityBatch);
} // namespace Scissio
