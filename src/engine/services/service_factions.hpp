#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(FactionData) {
    std::string id;
    std::string name;
    float color;

    SCHEMA_DEFINE(id, name, color);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("FactionData");
};

struct MessageFetchFactionRequest {
    std::string galaxyId;
    std::string systemId;

    MESSAGE_DEFINE(MessageFetchFactionRequest, galaxyId, systemId);
};

struct MessageFetchFactionResponse {
    FactionData faction;

    MESSAGE_DEFINE(MessageFetchFactionResponse, faction);
};

struct MessageFetchFactionsRequest {
    std::string galaxyId;
    std::string token;

    MESSAGE_DEFINE(MessageFetchFactionsRequest, galaxyId, token);
};

struct MessageFetchFactionsResponse {
    std::vector<FactionData> factions;
    std::string token;
    bool hasNext{false};

    MESSAGE_DEFINE(MessageFetchFactionsResponse, factions, token, hasNext);
};

class ServiceFactions : public Service {
public:
    explicit ServiceFactions(const Config& config, Registry& registry, TransactionalDatabase& db,
                             MsgNet::Server& server, Service::SessionValidator& sessionValidator);

    void create(const FactionData& faction);
    void handle(const PeerPtr& peer, MessageFetchFactionRequest req, MessageFetchFactionResponse& res);
    void handle(const PeerPtr& peer, MessageFetchFactionsRequest req, MessageFetchFactionsResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
