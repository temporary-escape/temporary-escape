#include "service_sectors.hpp"

using namespace Engine;

ServiceSectors::ServiceSectors(const Config& config, Registry& registry, TransactionalDatabase& db,
                               Network::Server& server, Service::SessionValidator& sessionValidator,
                               EventBus& eventBus) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator}, eventBus{eventBus} {

    HANDLE_REQUEST(MessageFetchSectorsRequest, MessageFetchSectorsResponse);
}

std::optional<SectorData> ServiceSectors::find(const std::string& galaxyId, const std::string& systemId,
                                               const std::string& sectorId) {
    return db.get<SectorData>(fmt::format("{}/{}/{}", galaxyId, systemId, sectorId));
}

void ServiceSectors::create(const SectorData& sector) {
    db.put(fmt::format("{}/{}/{}", sector.galaxyId, sector.systemId, sector.id), sector);
}

void ServiceSectors::handle(const Service::PeerPtr& peer, MessageFetchSectorsRequest req,
                            MessageFetchSectorsResponse& res) {
    (void)sessionValidator.find(peer);

    res.sectors = db.next<SectorData>(fmt::format("{}/{}/", req.galaxyId, req.systemId), req.token, 64, &res.token);
    res.hasNext = res.sectors.size() == 64;
}
