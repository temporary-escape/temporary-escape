#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
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

class ServiceSectors : public Service {
public:
    explicit ServiceSectors(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceSectors();

private:
    void handle(Request<MessageFetchSectorRequest> req);
    void handle(Request<MessageFetchSectorsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine