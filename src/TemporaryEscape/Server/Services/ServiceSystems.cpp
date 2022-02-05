#include "ServiceSystems.hpp"
#include "../../Utils/Random.hpp"

#define CMP "ServiceSystem"

using namespace Engine;

class GalaxyDistribution {
public:
    explicit GalaxyDistribution(const float maxWidth)
        : maxWidth(maxWidth), distArmIndex(0, ARMS_COUNT), distArmDistance(0.01f, 1.0f),
          distArmOffset(-ARM_ANGLE_HALF, ARM_ANGLE_HALF), distGridOffset(-0.1f, 0.1f) {
    }

    [[nodiscard]] Vector2 operator()(std::mt19937_64& rng) {
        Vector2i fixed;

        auto count = 5;
        while (count-- > 0) {
            const auto dist = distArmDistance(rng);
            auto angleOffset = (distArmOffset(rng) * (1.0f / dist));
            if (angleOffset > 0.0f) {
                angleOffset = std::pow(angleOffset, 2.0f);
            } else {
                angleOffset = -std::pow(-angleOffset, 2.0f);
            }
            auto angle = distArmIndex(rng) * ARM_ANGLE + angleOffset;

            angle = angle + dist * 2.0f;

            auto vec = Vector2{dist * maxWidth, 0.0f};
            const auto ca = std::cos(angle);
            const auto sa = std::sin(angle);
            vec = Vector2{ca * vec.x - sa * vec.y, sa * vec.x + ca * vec.y};

            fixed = Vector2i{vec};

            const auto test = *reinterpret_cast<uint64_t*>(&fixed.x);
            if (positions.find(test) == positions.end()) {
                positions.insert(test);
                break;
            }
        }

        Vector2 pos{fixed};
        pos = Vector2{pos.x + distGridOffset(rng), pos.y + distGridOffset(rng)};

        return pos;
    }

private:
    static constexpr auto ARMS_COUNT = 5;
    static constexpr auto ARM_ANGLE = static_cast<float>(2.0 * glm::pi<double>() / double(ARMS_COUNT));
    static constexpr auto ARM_ANGLE_HALF = ARM_ANGLE / 2.0f - ARM_ANGLE * 0.05f;

    float maxWidth;

    std::unordered_set<uint64_t> positions;
    std::uniform_int_distribution<int> distArmIndex;
    std::uniform_real_distribution<float> distArmDistance;
    std::uniform_real_distribution<float> distArmOffset;
    std::uniform_real_distribution<float> distGridOffset;
};

class ConnectionSolver {
public:
    using Result = std::unordered_map<size_t, std::vector<size_t>>;

    explicit ConnectionSolver(std::vector<Vector2> positions) : positions(std::move(positions)) {
    }

    Result solve() {
        struct WeightedConnection {
            float weight = 0.0f;
            size_t src = 0;
            size_t dst = 0;
        };

        const auto weightedComparator = [](const WeightedConnection& a, const WeightedConnection& b) -> bool {
            return a.weight > b.weight;
        };

        using WeightedConnections = std::vector<WeightedConnection>;
        using WeightedPriorityQueue =
            std::priority_queue<WeightedConnection, std::vector<WeightedConnection>, decltype(weightedComparator)>;

        std::vector<WeightedConnections> graph;
        graph.resize(positions.size());

        std::vector<std::vector<size_t>> source;
        source.resize(positions.size());

        const auto posToGridKey = [](const Vector2& pos) {
            const auto x = static_cast<int32_t>(std::roundf(pos.x / 20.0f));
            const auto y = static_cast<int32_t>(std::roundf(pos.y / 20.0f));
            const auto key =
                (static_cast<uint64_t>(x) & 0x0000FFFFULL) | ((static_cast<uint64_t>(y) & 0x0000FFFFULL) << 32ULL);
            return key;
        };

        // Distribute positions into a grid for a faster lookup of neighbour nodes
        std::unordered_map<uint64_t, std::vector<size_t>> cache;
        for (size_t i = 0; i < positions.size(); i++) {
            const auto& pos = positions.at(i);
            const auto key = posToGridKey(pos);
            cache[key].push_back(i);
        }

        const auto findNeighboursFast = [&](const size_t idx) {
            const auto& pos = positions.at(idx);
            std::vector<size_t> result;

            const auto x = static_cast<int32_t>(std::roundf(pos.x / 20.0f));
            const auto y = static_cast<int32_t>(std::roundf(pos.y / 20.0f));

            for (int32_t offX = -1; offX <= 1; offX++) {
                for (int32_t offY = -1; offY <= 1; offY++) {
                    const auto key = (static_cast<uint64_t>(x + offX) & 0x0000FFFFULL) |
                                     ((static_cast<uint64_t>(y + offY) & 0x0000FFFFULL) << 32ULL);

                    const auto found = cache.find(key);
                    if (found != cache.end()) {
                        const auto& nodes = found->second;
                        for (const auto& node : nodes) {
                            if (node != idx) {
                                result.push_back(node);
                            }
                        }
                    }
                }
            }

            return result;
        };

        const auto findNearestNeighbour = [&](const size_t idx) {
            WeightedConnection conn{};
            conn.weight = std::numeric_limits<float>::max();

            for (size_t i = 0; i < positions.size(); i++) {
                if (i == idx) {
                    continue;
                }

                const auto test = glm::distance2(positions.at(idx), positions.at(i));
                if (test < conn.weight) {
                    conn = {test, idx, i};
                }
            }

            return conn.dst;
        };

        // Add neighbours to all nodes in the graph
        for (size_t i = 0; i < positions.size(); i++) {
            const auto& pos = positions.at(i);
            const auto neighbours = findNeighboursFast(i);

            if (neighbours.empty()) {
                const auto nearest = findNearestNeighbour(i);
                const auto dist = glm::distance2(positions.at(i), positions.at(nearest));
                graph.at(i).push_back(WeightedConnection{dist, i, nearest});
                graph.at(nearest).push_back(WeightedConnection{dist, nearest, i});

            } else {
                auto& connections = graph.at(i);
                auto& sources = source.at(i);
                connections.reserve(neighbours.size());

                for (const auto& n : neighbours) {
                    const auto dist = glm::distance2(positions.at(i), positions.at(n));
                    connections.push_back(WeightedConnection{dist, i, n});
                }
            }
        }

        WeightedPriorityQueue q(weightedComparator);
        q.push(WeightedConnection{0.0f, 0, 0});

        std::vector<bool> added;
        added.resize(positions.size(), false);

        Result result;

        while (!q.empty()) {
            auto item = q.top();
            q.pop();

            const auto node = item.dst;

            if (!added.at(node)) {
                added[node] = true;

                if (item.src != node) {
                    result[item.src].push_back(node);
                }

                for (auto& pair : graph.at(node)) {
                    const auto adj = pair.dst;
                    if (!added.at(adj)) {
                        q.push(pair);
                    }
                }
            }
        }

        return result;
    }

private:
    std::vector<Vector2> positions;
};

ServiceSystems::ServiceSystems(const Config& config, AssetManager& assetManager, Engine::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServiceSystems::tick() {
}

std::vector<SystemData> ServiceSystems::getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                     const std::string& start, std::string& next) {
    return db.next<SystemData>(fmt::format("{}/", galaxyId), start, 64, &next);
}

std::vector<SectorPlanetData> ServiceSystems::getSystemPlanets(const std::string& galaxyId,
                                                               const std::string& systemId) {
    return db.seek<SectorPlanetData>(fmt::format("{}/{}/", galaxyId, systemId));
}

void ServiceSystems::generate() {
    const auto galaxies = db.seek<GalaxyData>("");
    for (const auto& galaxy : galaxies) {
        generate(galaxy.id);
    }
}

void ServiceSystems::generate(const std::string& galaxyId) {
    const auto& options = config.generator;

    Log::i(CMP, "Generating systems for galaxy: '{}' ...", galaxyId);

    const auto galaxyOpt = db.get<GalaxyData>(galaxyId);
    if (!galaxyOpt) {
        EXCEPTION("No such galaxy: '{}'", galaxyId);
    }

    const auto& galaxy = galaxyOpt.value();

    const auto test = db.seek<SystemData>(fmt::format("{}/", galaxy.id), 1);
    if (!test.empty()) {
        Log::i(CMP, "Already generated systems for galaxy: '{}' ...", galaxyId);
        return;
    }

    const auto regions = db.seek<RegionData>(fmt::format("{}/", galaxy.id));
    if (regions.empty()) {
        EXCEPTION("Number of regions in galaxy '{}' must not be zero", galaxy.id);
    }

    // Not optimized but the number of regions will be very low.
    // Should be around 18
    const auto getNearestRegion = [&](const Vector2& pos) {
        const RegionData* ptr = nullptr;
        for (const auto& region : regions) {
            if (ptr == nullptr) {
                ptr = &region;
            } else {
                const auto a = glm::distance(pos, region.pos);
                const auto b = glm::distance(pos, ptr->pos);

                if (a < b) {
                    ptr = &region;
                }
            }
        }

        assert(ptr != nullptr);
        return *ptr;
    };

    std::mt19937_64 rng(galaxy.seed);

    GalaxyDistribution distributor(options.galaxyWidth);

    std::unordered_set<std::string> namesTaken;
    namesTaken.reserve(options.totalSystems);

    std::vector<SystemData> systems;
    systems.reserve(options.totalSystems);

    std::vector<Vector2> positions;

    for (auto i = 0; i < options.totalSystems; i++) {
        const auto pos = distributor(rng);
        const auto region = getNearestRegion(pos);

        // System name
        std::string name;
        while (true) {
            const auto test = randomName(rng);
            if (namesTaken.find(test) == namesTaken.end()) {
                name = test;
                namesTaken.insert(test);
                break;
            }
        }

        SystemData system;
        system.id = uuid();
        system.galaxyId = galaxy.id;
        system.regionId = region.id;
        system.name = name;
        system.pos = pos;
        system.seed = randomInt<uint64_t>(rng);

        systems.push_back(system);
        positions.push_back(system.pos);
    }

    ConnectionSolver connectionSolver(std::move(positions));
    const auto connectionsMap = connectionSolver.solve();

    for (size_t i = 0; i < systems.size(); i++) {
        auto& system = systems.at(i);

        const auto connections = connectionsMap.find(i);
        if (connections != connectionsMap.end()) {
            for (const auto conn : connections->second) {
                auto& other = systems.at(conn);
                system.connections.push_back(other.id);
                other.connections.push_back(system.id);
            }
        }

       createSystem(system);
    }

    for (size_t i = 0; i < systems.size(); i++) {
        if (systems.at(i).connections.empty()) {
            Log::d(CMP, "System idx: {} has no connections", i);
        }
    }

    Log::i(CMP, "Generated {} systems for galaxy: '{}'", systems.size(), galaxy.name);
}

void ServiceSystems::createSystem(const SystemData& system) {
    db.put(fmt::format("{}/{}", system.galaxyId, system.id), system);
}
