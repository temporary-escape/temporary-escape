#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkTcpClient.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "../Server/Sector.hpp"
#include "../Utils/ReturnType.hpp"
#include "../Utils/Worker.hpp"
#include "../Utils/Yaml.hpp"
#include "Request.hpp"
#include "Stats.hpp"

namespace Engine {
struct PlayerLocalProfile {
    std::string name;
    uint64_t secret;

    YAML_DEFINE(name, secret);
};

class ENGINE_API Client : public NetworkTcpClient<Client, ServerSink> {
public:
    explicit Client(const Config& config, Registry& registry, Stats& stats, const Path& profilePath);
    virtual ~Client();

    Future<void> connect(const std::string& address, int port);
    void update();

    Scene* getScene() const {
        return scene.get();
    }

#if ENGINE_TESTS
    template <typename M> typename M::Response sendSync(typename M::Request& req) {
        auto promise = std::make_shared<Promise<typename M::Response>>();
        auto future = promise->future();

        NetworkTcpClient<Client, ServerSink>::template send(
            req, [promise](typename M::Response res) { promise->resolve(std::move(res)); });

        if (future.template waitFor(std::chrono::milliseconds(1000)) != std::future_status::ready) {
            EXCEPTION("Failed to wait for response of type: '{}'", typeid(typename M::Response).name());
        }
        return future.get();
    }

    template <typename Fn, typename R = return_type_t<Fn>> R check(Fn&& fn) {
        auto promise = std::make_shared<Promise<R>>();
        auto future = promise->future();

        sync.post([promise, fn = std::forward<Fn>(fn)] {
            try {
                promise->resolve(fn());
            } catch (std::exception_ptr& eptr) {
                promise->reject(eptr);
            }
        });

        if (future.template waitFor(std::chrono::milliseconds(1000)) != std::future_status::ready) {
            EXCEPTION("Failed to wait for promise to complete");
        }
        return future.get();
    }
#endif

    const std::string& getPlayerId() const {
        return playerId;
    }

    const PlayerLocationData& getPlayerLocation() const {
        return playerLocation;
    }

    template <typename M, typename Fn> void send(M& message, Fn&& callback) {
        NetworkTcpClient<Client, ServerSink>::template send(message, std::forward<Fn>(callback));
        ++stats.network.packetsSent;
    }

    void handle(MessagePlayerLocationChanged::Response res);
    void handle(MessageSceneEntities::Response res);
    void handle(MessageSceneDeltas::Response res);

private:
    void fetchModInfo(std::shared_ptr<Promise<void>> promise);
    void fetchLogin(std::shared_ptr<Promise<void>> promise);
    void fetchSpawnRequest(std::shared_ptr<Promise<void>> promise);

    Registry& registry;
    Stats& stats;
    PlayerLocalProfile localProfile;
    std::string playerId;
    PlayerLocationData playerLocation;

    // Promise<void> loggedIn;

    asio::io_service& sync;
    // PeriodicWorker worker1s{std::chrono::milliseconds(1000)};

    std::unique_ptr<Scene> scene;
    std::shared_ptr<Entity> camera;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;
};
} // namespace Engine
