#pragma once

#include "../Scene/Scene.hpp"
#include "../Utils/Worker.hpp"
#include "Generator.hpp"
#include "Messages.hpp"
#include "Session.hpp"

namespace Scissio {
class Server;

class Sector : public Scene::EventListener {
public:
    struct EntityView {
        EntityPtr ptr;
        bool sync{true};
    };

    explicit Sector(Server& server, Database& db, std::string compoundId);
    virtual ~Sector();

    void load(Generator& generator);
    void update();

    void addSession(SessionPtr session);

    void eventEntityAdded(const EntityPtr& entity) override;

private:
    struct SessionView {
        SessionPtr ptr;
        std::vector<EntityView> entities;
    };

    Server& server;
    Database& db;
    std::string compoundId;

    Scene scene;

    std::list<SessionView> sessions;

    asio::io_service sync;
};
} // namespace Scissio
