#pragma once

#include "../assets/registry.hpp"
#include "../config.hpp"
#include "../database/database.hpp"
#include "../network/server.hpp"
#include "../utils/event_bus.hpp"

#define HANDLE_REQUEST(Req, Res)                                                                                       \
    server.addHandler([this](const PeerPtr& peer, Req req) -> Res {                                                    \
        Res res{};                                                                                                     \
        this->handle(peer, std::move(req), res);                                                                       \
        return res;                                                                                                    \
    });

namespace Engine {
class ENGINE_API Service {
public:
    using PeerPtr = std::shared_ptr<Network::Peer>;

    class Session {
    public:
        virtual ~Session() = default;

        virtual const std::string& getPlayerId() const = 0;
    };

    class SessionValidator {
    public:
        virtual ~SessionValidator() = default;

        virtual std::shared_ptr<Session> find(const std::shared_ptr<Network::Peer>& peer) = 0;
    };

    virtual ~Service() = default;
};
} // namespace Engine
