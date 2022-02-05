#pragma once

#include "../Scene/Scene.hpp"
#include "../Utils/Worker.hpp"
#include "Messages.hpp"
#include "Player.hpp"

namespace Engine {
class Server;

class Sector : public Scene::EventListener {
public:
    struct EntityView {
        EntityPtr ptr;
        bool sync{true};
    };

    explicit Sector(Server& server, Database& db, std::string compoundId);
    virtual ~Sector();

    void load();
    void update();

    void addPlayer(PlayerPtr player);

    void eventEntityAdded(const EntityPtr& entity) override;

    const std::string& getCompoundId() const {
        return compoundId;
    }

private:
    struct PlayerView {
        PlayerPtr ptr;
        std::vector<EntityView> entities;
    };

    Server& server;
    Database& db;
    std::string compoundId;
    bool loaded;
    Scene scene;

    std::list<PlayerView> players;

    asio::io_service sync;
};

using SectorPtr = std::shared_ptr<Sector>;
} // namespace Engine
