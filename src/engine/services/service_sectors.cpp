#include "service_sectors.hpp"

using namespace Engine;

ServiceSectors::ServiceSectors(const Config& config, Registry& registry, TransactionalDatabase& db,
                               MsgNet::Server& server, Service::SessionValidator& sessionValidator) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator} {
}

std::optional<SectorData> ServiceSectors::find(const std::string& galaxyId, const std::string& systemId,
                                               const std::string& sectorId) {
    return db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
}

void ServiceSectors::create(const SectorData& sector) {
    db.put(fmt::format("{}/{}/{}", sector.galaxyId, sector.systemId, sector.id), sector);
}
