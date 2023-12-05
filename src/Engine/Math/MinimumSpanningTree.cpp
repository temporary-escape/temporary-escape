#include "MinimumSpanningTree.hpp"
#include "../Utils/Random.hpp"
#include <glm/gtx/norm.hpp>
#include <queue>

using namespace Engine;

void MinimumSpanningTree::calculate() {
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

    while (!q.empty()) {
        auto item = q.top();
        q.pop();

        const auto node = item.dst;

        if (!added.at(node)) {
            added[node] = true;

            if (item.src != node) {
                results[item.src].push_back(node);
            }

            for (auto& pair : graph.at(node)) {
                const auto adj = pair.dst;
                if (!added.at(adj)) {
                    q.push(pair);
                }
            }
        }
    }
}
