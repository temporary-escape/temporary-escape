#include "Pathfinding.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"

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
static inline uint8_t idxToMask(const Args... args) {
    uint8_t result{0};
    ((result |= 1 << args), ...);
    return result;
}

static std::array<uint8_t, 6> sidesMask = {
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
    const auto start = findNearest(from);
    if (!start) {
        return false;
    }

    const auto goal = findNearest(to);
    if (!goal) {
        return false;
    }

    logger.info("Start pos: {} level: {} code: {:x}", start.pos, start.level, start.code);
    logger.info("Goal pos: {} level: {} code: {:x}", goal.pos, goal.level, goal.code);

    getNeighbours(start);

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
            nodes[i].offset += start + temp[level].size();
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

void Pathfinding::getNeighbours(const NodeInfo& info) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - info.level)));
    const auto childWidth = nodeWidth / 2;

    const auto found = findNearest(info.pos + Vector3i{nodeWidth, 0, 0}, info.level);
    logger.info("Found nearest -x code: {:x} pos: {} level: {}", found.code, found.pos, found.level);

    /*std::array<Vector3i, 6> neighbours = {
        info.pos + Vector3i{childWidth, 0, 0},
        info.pos + Vector3i{0, childWidth, 0},

        info.pos + Vector3i{-childWidth, 0, 0},
        info.pos + Vector3i{0, -childWidth, 0},

        info.pos + Vector3i{0, 0, childWidth},
        info.pos + Vector3i{0, 0, -childWidth},
    };*/
}

void Pathfinding::getNeighboursSide(const Vector3i& pos, const uint8_t side) {
    logger.info("Looking for neighbours at: {} side: {}", pos, side);
    getNeighboursSide(0, {0, 0, 0}, 0, HashCode{0}, pos, side);
}

void Pathfinding::getNeighboursSide(const Index index, const Vector3i& origin, const int level, const HashCode previous,
                                    const Vector3i& pos, const uint8_t side) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    const auto childWidth = nodeWidth / 2;

    auto& node = nodes[index];

    for (int idx = 0; idx < 8; idx++) {
        if (node.children & (1 << idx) && sidesMask[side]) {
            const auto nextCode = previous | (((idx + 1) & 0x0F) << (level * 4));
            const auto childOrigin = idxToOffset(idx, origin, nodeWidth);


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
