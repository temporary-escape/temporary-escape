#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(PlayerData) {
    std::string id;
    uint64_t secret;
    std::string name;
    bool admin;

    SCHEMA_DEFINE(id, secret, name, admin);
    SCHEMA_INDEXES(secret);
    SCHEMA_NAME("PlayerData");
};

SCHEMA(PlayerLocationData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    SCHEMA_DEFINE(id, galaxyId, systemId, sectorId);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("PlayerLocationData");
};

SCHEMA(PlayerUnlockedBlocks) {
    std::string id;
    std::vector<BlockPtr> blocks;

    SCHEMA_DEFINE(id, blocks);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("PlayerUnlockedBlocks");
};

struct MessagePlayerLocationRequest {
    bool dummy{false};

    MESSAGE_DEFINE(MessagePlayerLocationRequest, dummy);
};

struct MessagePlayerLocationResponse {
    std::string galaxyId;
    std::string systemId;
    std::string sectorId;

    MESSAGE_DEFINE(MessagePlayerLocationResponse, galaxyId, systemId, sectorId);
};

class ServicePlayers : public Service {
public:
    explicit ServicePlayers(const Config& config, Registry& registry, TransactionalDatabase& db, MsgNet::Server& server,
                            Service::SessionValidator& sessionValidator);

    std::optional<std::string> secretToId(uint64_t);
    PlayerData login(uint64_t secret, const std::string& name);
    PlayerLocationData findStartingLocation(const std::string& playerId);
    PlayerLocationData getLocation(const std::string& playerId);
    std::vector<BlockPtr> getUnlockedBlocks(const std::string& playerId);

    void handle(const PeerPtr& peer, MessagePlayerLocationRequest req, MessagePlayerLocationResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
