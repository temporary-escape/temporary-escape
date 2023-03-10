#include "service_factions.hpp"

using namespace Engine;

ServiceFactions::ServiceFactions(const Config& config, Registry& registry, TransactionalDatabase& db,
                                 Network::Server& server, Service::SessionValidator& sessionValidator,
                                 EventBus& eventBus) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator}, eventBus{eventBus} {

    HANDLE_REQUEST(MessageFetchFactionRequest, MessageFetchFactionResponse);
    HANDLE_REQUEST(MessageFetchFactionsRequest, MessageFetchFactionsResponse);
}

std::vector<FactionData> ServiceFactions::get() {
    return db.seekAll<FactionData>("");
}

void ServiceFactions::create(const FactionData& faction) {
    db.put(fmt::format("{}", faction.id), faction);
}

void ServiceFactions::handle(const PeerPtr& peer, MessageFetchFactionRequest req, MessageFetchFactionResponse& res) {
    (void)sessionValidator.find(peer);

    const auto found = db.get<FactionData>(fmt::format("{}", req.galaxyId, req.systemId));
    if (!found) {
        EXCEPTION("No such faction: {}", req.galaxyId, req.systemId);
    }

    res.faction = *found;
}

void ServiceFactions::handle(const PeerPtr& peer, MessageFetchFactionsRequest req, MessageFetchFactionsResponse& res) {
    (void)sessionValidator.find(peer);

    res.factions = db.next<FactionData>("", req.token, 64, &res.token);
    res.hasNext = res.factions.size() == 64;
}
