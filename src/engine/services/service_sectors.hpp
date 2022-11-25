#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(SectorData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;
    bool generated = false;

    SCHEMA_DEFINE(id, galaxyId, systemId, name, pos, seed, generated);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SectorData");
};

class ServiceSectors : public Service {
public:
    explicit ServiceSectors(const Config& config, Registry& registry, TransactionalDatabase& db, MsgNet::Server& server,
                            Service::SessionValidator& sessionValidator);

    std::optional<SectorData> find(const std::string& galaxyId, const std::string& systemId,
                                   const std::string& sectorId);
    void create(const SectorData& sector);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
