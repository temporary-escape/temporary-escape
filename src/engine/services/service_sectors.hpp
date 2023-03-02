#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(SectorData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    Vector2 pos{0.0f, 0.0f};
    uint64_t seed{0};
    bool generated{false};

    SCHEMA_DEFINE(id, galaxyId, systemId, name, pos, seed, generated);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SectorData");
};

struct MessageFetchSectorsRequest {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MESSAGE_DEFINE(MessageFetchSectorsRequest, galaxyId, systemId, token);
};

struct MessageFetchSectorsResponse {
    std::vector<SectorData> sectors;
    std::string token;
    bool hasNext{false};

    MESSAGE_DEFINE(MessageFetchSectorsResponse, sectors, token, hasNext);
};

class ENGINE_API ServiceSectors : public Service {
public:
    explicit ServiceSectors(const Config& config, Registry& registry, TransactionalDatabase& db,
                            Network::Server& server, Service::SessionValidator& sessionValidator, EventBus& eventBus);

    std::optional<SectorData> find(const std::string& galaxyId, const std::string& systemId,
                                   const std::string& sectorId);
    void create(const SectorData& sector);

    void handle(const PeerPtr& peer, MessageFetchSectorsRequest req, MessageFetchSectorsResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
    EventBus& eventBus;
};
} // namespace Engine
