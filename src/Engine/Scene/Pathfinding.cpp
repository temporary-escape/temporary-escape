#include "Pathfinding.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include <queue>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

template <typename V>
static inline glm::vec<3, V> idxToOffset(const int idx, const glm::vec<3, V>& origin, const V size) {
    using Vec = glm::vec<3, V>;

    const auto off = size / 4.0f;

    switch (idx) {
    case 0: {
        return origin + Vec{off, off, off};
    }
    case 1: {
        return origin + Vec{-off, off, off};
    }
    case 2: {
        return origin + Vec{-off, off, -off};
    }
    case 3: {
        return origin + Vec{off, off, -off};
    }
    case 4: {
        return origin + Vec{off, -off, off};
    }
    case 5: {
        return origin + Vec{-off, -off, off};
    }
    case 6: {
        return origin + Vec{-off, -off, -off};
    }
    case 7: {
        return origin + Vec{off, -off, -off};
    }
    default: {
        throw std::out_of_range("Invalid index");
    }
    }
}

template <typename... Args>
static inline constexpr uint8_t idxToMask(const Args... args) {
    uint8_t result{0};
    ((result |= 1 << args), ...);
    return result;
}

static constexpr std::array<uint8_t, 6> sidesMask = {
    // Positive X
    idxToMask(0, 3, 4, 7),
    // Negative X
    idxToMask(1, 2, 5, 6),
    // Positive Y
    idxToMask(0, 1, 2, 3),
    // Negative Y
    idxToMask(4, 5, 6, 7),
    // Positive Z
    idxToMask(0, 1, 4, 5),
    // Negative Z
    idxToMask(2, 3, 6, 7),
};

static inline int getHashCodeLevel(const Pathfinding::HashCode code) {
    int level = 0;
    while (true) {
        const auto test = 0x0FULL << (level * 4ULL);
        if (!(test & code)) {
            break;
        }
        ++level;
    }
    return level - 1;
}

static inline bool isInsideBox(const Vector3i& origin, const int width, const Vector3i& pos) {
    const auto half = width / 2;

    if (pos.x >= origin.x - half && pos.x < origin.x + half && pos.y >= origin.y - half && pos.y < origin.y + half &&
        pos.z >= origin.z - half && pos.z < origin.z + half) {
        return true;
    }

    return false;
}

static int64_t heuristic(const Vector3i& a, const Vector3i& b) {
    return glm::abs(a.x - b.x) + glm::abs(a.y - b.y) + glm::abs(a.z - b.z);
}

Pathfinding::Pathfinding(Tester& tester, const int depth, const int scale) :
    tester{tester}, depth{depth}, scale{scale}, levels{} {

    // Create all layers
    for (auto d = 0; d < depth; d++) {
        temp.emplace_back();
    }

    // Create the root node
    auto& root = temp[0].emplace_back();
    root.offset = 0;
    root.children = 0;
}

void Pathfinding::build() {
    logger.info("Pathfinding building of size: {} units", std::pow(2, depth) * scale);
    const auto t0 = std::chrono::high_resolution_clock::now();
    build(0, {0, 0, 0}, 0);
    optimize();
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    logger.info("Pathfinding built in {}ms with {} nodes", diff.count(), nodes.size());
}

std::optional<Pathfinding::HashCode> Pathfinding::find(const Vector3& pos) {
    return find(0, {0, 0, 0}, 0, HashCode{0}, pos);
}

Pathfinding::NodeInfo Pathfinding::findNearest(const Vector3& pos, int maxLevel) {
    return findNearest(0, {0, 0, 0}, 0, HashCode{0}, pos, maxLevel);
}

bool Pathfinding::findPath(const Vector3& from, const Vector3& to) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    QueryData data;

    data.start = findNearest(from);
    if (!data.start) {
        return false;
    }

    data.goal = findNearest(to);
    if (!data.goal) {
        return false;
    }

    logger.info("Start pos: {} level: {} code: {:x}", data.start.pos, data.start.level, data.start.code);
    logger.info("Goal pos: {} level: {} code: {:x}", data.goal.pos, data.goal.level, data.goal.code);

    /*if (infos.size() != nodes.size()) {
        infos.resize(nodes.size());
    }
    std::memset(infos.data(), 0, infos.size() * sizeof(Info));*/

    // getNeighbours(start);

    std::vector<NodeInfoDistance> frontierVec;
    frontierVec.reserve(1 * 1024);
    data.frontier = std::priority_queue<NodeInfoDistance>(std::less<NodeInfoDistance>(), std::move(frontierVec));
    data.frontier.emplace(NodeInfoDistance{data.start, 0});

    data.costSoFar.reserve(1 * 1024);
    data.costSoFar[data.start.code] = Info{.from = 0, .cost = 0,};

    size_t count{0};

    while (!data.frontier.empty()) {
        data.current = data.frontier.top();
        data.frontier.pop();

        if (data.current.info.code == data.goal.code) {
            break;
        }

        getNeighbours(data.current.info, data);
    }

    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    logger.info("Pathfinding found in {}ms count: {}", diff.count(), count);

    return true;
}

void Pathfinding::optimize() {
    for (int level = 0; level < depth; level++) {
        // Allocate the level into the continuous array
        const auto start = allocateNodes(temp[level].size());

        logger.debug(
            "Pathfinding optimize copying level: {} to offset: {} of size: {}", level, start, temp[level].size());

        // Copy the nodes over
        std::memcpy(&nodes[start], &temp[level][0], temp[level].size() * sizeof(Node));

        // Update the offsets in this layer
        // The start of the next layer is exactly at the end of this layer
        for (size_t i = start; i < start + temp[level].size(); i++) {
            //if (nodes[i].offset) {
                nodes[i].offset += start + temp[level].size();
            //}
        }
    }

    // Free the temp nodes memory
    temp.clear();
    temp.shrink_to_fit();
}

void Pathfinding::build(Index index, const Vector3i& origin, const int level) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    if (nodeWidth < 4) {
        EXCEPTION("Pathfinding node width is too small at level: {}", level);
    }

    const auto childWidth = nodeWidth / 2;

    ChildrenMask children{0};
    for (int idx = 0; idx < 8; idx++) {
        const auto childOrigin = idxToOffset<int>(idx, origin, nodeWidth);

        if (tester.contactTestBox(childOrigin, childWidth)) {
            children |= 1 << idx;
        }
    }

    auto& node = temp[level][index];
    node.children = children;
    node.offset = 0;

    if (children && level + 1 != depth) {
        node.offset = allocateTempNodes(level + 1);

        for (int idx = 0; idx < 8; idx++) {
            if (children & (1 << idx)) {
                const auto childOrigin = idxToOffset<int>(idx, origin, nodeWidth);
                build(node.offset + idx, childOrigin, level + 1);
            } else {
                auto& child = temp[level + 1][node.offset + idx];
                child.offset = 0;
                child.children = 0;
            }
        }
    }
}

Pathfinding::Index Pathfinding::allocateNodes(const size_t num) {
    if (nodes.size() + num > nodes.capacity()) {
        nodes.reserve(nodes.capacity() + nodeCapacityMultipler);
    }

    const auto offset = nodes.size();
    nodes.resize(offset + num);
    return static_cast<Index>(offset);
}

Pathfinding::Index Pathfinding::allocateTempNodes(const int level) {
    if (level <= 0 || level >= temp.size()) {
        EXCEPTION("Failed to allocate nodes, error: level {} out of bounds", level);
    }

    auto& layer = temp[level];
    if (layer.size() + 8 > layer.capacity()) {
        const auto expectedNodes = static_cast<size_t>(std::pow(2, level - 1));
        layer.reserve(layer.size() + std::min<size_t>(nodeCapacityMultipler, expectedNodes));
    }

    const auto offset = layer.size();
    layer.resize(layer.size() + 8);
    return static_cast<Index>(offset);
}

void Pathfinding::getNeighbours(const NodeInfo& info, QueryData& data) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - info.level)));

    std::array<NodeInfo, 6> neighbours = {
        findNearest(info.pos + Vector3i{nodeWidth, 0, 0}, info.level),
        findNearest(info.pos + Vector3i{-nodeWidth, 0, 0}, info.level),
        findNearest(info.pos + Vector3i{0, nodeWidth, 0}, info.level),
        findNearest(info.pos + Vector3i{0, -nodeWidth, 0}, info.level),
        findNearest(info.pos + Vector3i{0, 0, nodeWidth}, info.level),
        findNearest(info.pos + Vector3i{0, 0, -nodeWidth}, info.level)
    };

    for (size_t i = 0; i < neighbours.size(); i++) {
        if (!neighbours[i]) {
            continue;
        }

        if (neighbours[i].level == depth) {
            processNeighbour(neighbours[i], data);
            continue;
        }

        auto side = static_cast<int>(i);
        if (side % 2 == 0) {
            ++side;
        } else {
            --side;
        }
        /*if (neighbour) {
            logger.info("Found neighbour code: {:x} pos: {} level: {}", neighbour.code, neighbour.pos, neighbour.level);
        }*/
        const auto& n = neighbours[i];
        getNeighboursSide(n.offset, n.pos, n.level, n.code, side, data);
    }

    //const auto& n = neighbours[0];
    // getNeighboursSide(n.offset, n.pos, n.level, n.code, 1, callback);
}

void Pathfinding::processNeighbour(const NodeInfo& next, QueryData& data) {
    const auto newCost = data.costSoFar[data.current.info.code].cost + heuristic(data.current.info.pos, next.pos);
    if (data.costSoFar.find(next.code) == data.costSoFar.end() || newCost < data.costSoFar[next.code].cost) {
        data.costSoFar[next] = Info{.from = data.current.info.code, .cost = newCost,};
        const auto priority = newCost + heuristic(next.pos, data.goal.pos);
        data.frontier.push(NodeInfoDistance{next, priority});
    }
}

void Pathfinding::getNeighboursSide(const Index index, const Vector3i& origin, const int level, const HashCode previous,
                                    const uint8_t side, QueryData& data) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    if (level + 1 == depth || !nodes[index].children) {
        processNeighbour(NodeInfo{
            .pos = origin,
            .offset = index,
            .code = previous,
            .level = level,
        }, data);
        return;
    }

    auto& node = nodes[index];

    for (int idx = 0; idx < 8; idx++) {
        if (sidesMask[side] & (1 << idx)) {
            const auto nextCode = previous | (((idx + 1) & 0x0F) << (level * 4));
            const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

            if (node.children & (1 << idx) && level + 1 < depth) {
                getNeighboursSide(node.offset + idx, childOrigin, level + 1, nextCode, side, data);
            } else if (node.children & (1 << idx)) {
                // Occupied space
                continue;
            } else {
                // Empty space
                processNeighbour(NodeInfo{
                    .pos = childOrigin,
                    .offset = level + 1 == depth ? 0 : static_cast<Index>(node.offset + idx),
                    .code = nextCode,
                    .level = level + 1,
                }, data);
            }
        }
    }
}

std::optional<Pathfinding::HashCode> Pathfinding::find(const Index index, const Vector3i& origin, const int level,
                                                       const HashCode previous, const Vector3& pos) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    const auto childWidth = nodeWidth / 2;

    auto& node = nodes[index];

    for (int idx = 0; idx < 8; idx++) {
        if (node.children & (1 << idx)) {
            const auto nextCode = previous | (((idx + 1) & 0x0F) << (level * 4));
            const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

            if (isInsideBox(childOrigin, childWidth, pos)) {
                if (level + 1 == depth) {
                    return nextCode;
                } else {
                    const auto test = find(node.offset + idx, childOrigin, level + 1, nextCode, pos);
                    if (test) {
                        return test;
                    }
                }
            }
        }
    }

    return std::nullopt;
}

Pathfinding::NodeInfo Pathfinding::findNearest(const Index index, const Vector3i& origin, const int level,
                                             const HashCode previous, const Vector3& pos, int maxLevel) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    const auto childWidth = nodeWidth / 2;

    auto& node = nodes[index];

    for (int idx = 0; idx < 8; idx++) {
        const auto nextCode = previous | HashCode{(idx & 0x0FULL) << (level * 4ULL)};
        const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

        if (isInsideBox(childOrigin, childWidth, pos)) {
            if (level + 1 != depth && node.children & (1 << idx) && level + 1 < maxLevel) {
                return findNearest(node.offset + idx, childOrigin, level + 1, nextCode, pos, maxLevel);
            } else {
                return NodeInfo{
                    .pos = childOrigin,
                    .offset = static_cast<Index>(node.offset + idx),
                    .code = nextCode,
                    .level = level + 1,
                };
            }
        }
    }

    return {
        .pos = {0, 0, 0},
        .offset = 0,
        .code = 0,
        .level = 0,
    };
}
