#include "Pathfinding.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

#define NODE_ADDRESS_MASK 0xFFFFFFULL
#define NODE_CHILD_OFFSET 0ULL
#define NODE_CHILD_MASK (NODE_ADDRESS_MASK << NODE_CHILD_OFFSET)
#define NODE_NEXT_OFFSET 24ULL
#define NODE_NEXT_MASK (NODE_ADDRESS_MASK << NODE_NEXT_OFFSET)
#define NODE_INDEX_OFFSET 48ULL
#define NODE_INDEX_MASK (0x07ULL << NODE_INDEX_OFFSET)

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

static inline bool isInsideBox(const Vector3i& origin, const int width, const Vector3& pos) {
    const auto half = width / 2;

    if (pos.x > origin.x - half && pos.x < origin.x + half && pos.y > origin.y - half && pos.y < origin.y + half &&
        pos.z > origin.z - half && pos.z < origin.z + half) {
        return true;
    }

    return false;
}

Pathfinding::Pathfinding(Tester& tester, const int depth, const int scale) :
    tester{tester}, depth{depth}, scale{scale}, count{0} {

    addNode();
}

size_t Pathfinding::addNode() {
    for (size_t b = 0; b < buckets.size(); b++) {
        auto& bucket = buckets.at(b);
        const auto offset = b * bucketSize;

        // Does the bucket have free space?
        if (bucket.used < bucketSize) {
            // Fast check if the next (a suggestion) is an empty node
            if (nodes.at(offset + bucket.next) == 0) {
                bucket.used++;
                count++;
                return offset + bucket.next++;
            }

            // Find an empty node in the bucket
            for (size_t i = 0; i < bucketSize; i++) {
                if (nodes.at(offset + i) == 0) {
                    bucket.next = offset + i + 1;
                    count++;
                    bucket.used++;
                    return offset + i;
                }
            }
        }
    }

    // Buckets are full, add a new one
    nodes.resize(nodes.capacity() + bucketSize);
    const auto offset = buckets.size() * bucketSize;
    auto& bucket = buckets.emplace_back();
    count++;
    bucket.used++;
    bucket.next = 1;
    return nodes.at(offset + 0);
}

void Pathfinding::build() {
    build(0, {0, 0, 0}, 0);
}

size_t Pathfinding::find(const Vector3& pos) {
    return find(0, {0, 0, 0}, 0, pos);
}

void Pathfinding::iterate(const std::function<void(const Vector3i&, int)>& fn) {
    return iterate(0, {0, 0, 0}, 0, fn);
}

inline void Pathfinding::setNodeValue(const size_t node, const uint64_t offset, const uint64_t mask,
                                      const uint64_t value) {
    nodes.at(node) = (nodes.at(node) & ~mask) | ((value << offset) & mask);
}

inline uint64_t Pathfinding::getNodeValue(const size_t node, const uint64_t offset, const uint64_t mask) {
    return (nodes.at(node) & mask) >> offset;
}

void Pathfinding::build(size_t node, const Vector3i& origin, const int level) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));
    /*logger.info("Building node: {} origin: {} level: {} width: {}",
                reinterpret_cast<uint64_t>(&node),
                origin,
                level,
                nodeWidth);*/

    const auto childWidth = nodeWidth / 2;

    auto previous = node;

    for (int idx = 0; idx < 8; idx++) {
        const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

        if (tester.contactTestBox(childOrigin, static_cast<float>(childWidth))) {
            // logger.info("Contact test success origin: {} width: {}", childOrigin, childWidth);
            const auto child = addNode();

            // Is this the first child?
            if (previous == node) {
                setNodeValue(previous, NODE_CHILD_OFFSET, NODE_CHILD_MASK, child);
                previous = child;
            }
            // Must be a neighbour
            else {
                setNodeValue(previous, NODE_NEXT_OFFSET, NODE_NEXT_MASK, child);
                previous = child;
            }

            setNodeValue(previous, NODE_INDEX_OFFSET, NODE_INDEX_MASK, idx);

            /*if (level + 1 == depth) {
                logger.info("Leaf origin: {} width: {}", childOrigin, childWidth);
            }*/ /* else {
                 logger.info("Branch origin: {} width: {}", childOrigin, childWidth);
             }*/
        }
    }

    if (level + 1 < depth) {
        auto child = getNodeValue(node, NODE_CHILD_OFFSET, NODE_CHILD_MASK);

        while (child) {
            const auto idx = static_cast<int>(getNodeValue(child, NODE_INDEX_OFFSET, NODE_INDEX_MASK));
            const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

            build(child, childOrigin, level + 1);

            child = getNodeValue(child, NODE_NEXT_OFFSET, NODE_NEXT_MASK);
        }
    }
}

size_t Pathfinding::find(size_t node, const Vector3i& origin, const int level, const Vector3& pos) {
    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    const auto childWidth = nodeWidth / 2;

    auto child = getNodeValue(node, NODE_CHILD_OFFSET, NODE_CHILD_MASK);

    while (child) {
        const auto idx = static_cast<int>(getNodeValue(child, NODE_INDEX_OFFSET, NODE_INDEX_MASK));
        const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

        if (isInsideBox(childOrigin, childWidth, pos)) {
            if (level + 1 == depth) {
                return child;
            } else {
                const auto test = find(child, childOrigin, level + 1, pos);
                if (test) {
                    return test;
                }
            }
        }

        child = getNodeValue(child, NODE_NEXT_OFFSET, NODE_NEXT_MASK);
    }

    /*for (int idx = 0; idx < 8; idx++) {
        const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

        auto* child = node.children[idx];
        if (!child) {
            continue;
        }

        if (isInsideBox(childOrigin, childWidth, pos)) {
            if (level + 1 == depth) {
                return child;
            } else {
                const auto test = find(*child, childOrigin, level + 1, pos);
                if (test) {
                    return test;
                }
            }
        }
    }*/

    return 0;
}

void Pathfinding::iterate(size_t node, const Vector3i& origin, int level,
                          const std::function<void(const Vector3i&, int)>& fn) {

    const auto nodeWidth = scale * static_cast<int>(std::pow(2.0f, static_cast<float>(depth - level)));

    const auto childWidth = nodeWidth / 2;

    auto child = getNodeValue(node, NODE_CHILD_OFFSET, NODE_CHILD_MASK);

    while (child) {
        const auto idx = static_cast<int>(getNodeValue(child, NODE_INDEX_OFFSET, NODE_INDEX_MASK));
        const auto childOrigin = idxToOffset(idx, origin, nodeWidth);

        if (level + 1 == depth) {
            fn(childOrigin, childWidth);
        } else {
            iterate(child, childOrigin, level + 1, fn);
        }

        child = getNodeValue(child, NODE_NEXT_OFFSET, NODE_NEXT_MASK);
    }
}
