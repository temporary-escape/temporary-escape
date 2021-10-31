#pragma once

#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Msgpack.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <vector>

namespace Scissio {
template <typename V> inline glm::vec<3, V> idxToOffset(const int idx, const V s, const glm::vec<3, V>& origin) {
    using Vec = glm::vec<3, V>;

    switch (idx) {
    case 0: {
        return origin + Vec{s, s, s};
    }
    case 1: {
        return origin + Vec{-s, s, s};
    }
    case 2: {
        return origin + Vec{-s, s, -s};
    }
    case 3: {
        return origin + Vec{s, s, -s};
    }
    case 4: {
        return origin + Vec{s, -s, s};
    }
    case 5: {
        return origin + Vec{-s, -s, s};
    }
    case 6: {
        return origin + Vec{-s, -s, -s};
    }
    case 7: {
        return origin + Vec{s, -s, -s};
    }
    default: {
        throw std::out_of_range("Invalid index");
    }
    }
}

template <typename T> class Octree {
public:
    static constexpr uint16_t INVALID_PARENT = 0xFFFF;

    struct Node {
        uint16_t parent{INVALID_PARENT};
        std::array<uint16_t, 8> children{0};
        T data{};

        bool isBranch() const {
            for (const auto& child : children) {
                if (child != 0) {
                    return true;
                }
            }
            return false;
        }

        bool isLeaf() const {
            return !isBranch();
        }

        MSGPACK_DEFINE_ARRAY(parent, children, data);
    };

    struct NodeRef {
        Node& node;
        uint16_t offset;
    };

    struct RayCastResut {
        std::reference_wrapper<Node> node;
        uint16_t offset;
        Vector3 pos;
    };

    Octree() = default;
    virtual ~Octree() = default;

    NodeRef insert(const Vector3& pos) {
        const auto position = align(pos);

        if (isEmpty()) {
            auto& root = nextEmptyNode();
            root.parent = 0;
            width = 2;
        }

        while (isOutside(position)) {
            expand(0);
        }

        return insert(nodes.at(0), Vector3i{0}, width / 2, position);
    }

    std::optional<NodeRef> find(const Vector3& pos) {
        const auto position = align(pos);

        return find(nodes.at(0), Vector3i{0}, width / 2, position);
    }

    std::optional<RayCastResut> rayCast(const Vector3& from, const Vector3& to) {

        return rayCast(nodes.at(0), Vector3i{0}, width / 2, from, to);
    }

    void remove(const Node& node) {
        if (&node < &nodes.front() || &node > &nodes.back()) {
            // Not our node or the node reference is old
            return;
        }

        // Check if the parent exists
        if (node.parent != INVALID_PARENT) {
            // Remove this node from its parent
            clearChildOf(nodes.at(node.parent), node);
        }

        const auto offset = &node - &nodes.front();

        auto& found = nodes.at(offset);
        found.parent = INVALID_PARENT;
        found.data = T{};

        if (offset < next) {
            next = offset;
        }
    }

    void remove(const uint16_t offset) {
        if (offset < nodes.size()) {
            const auto& node = nodes.at(offset);
            remove(node);
        }
    }

    Node& get(const uint16_t offset) {
        return nodes.at(offset);
    }

    const std::vector<Node>& getNodes() const {
        return nodes;
    }

    Vector3 getBoundingBox() const {
        return Vector3{static_cast<float>(width)};
    }

    uint16_t getSize() const {
        return nodes.size();
    }

    bool isEmpty() const {
        return nodes.empty() || nodes.at(0).parent == INVALID_PARENT;
    }

    Node& root() {
        return get(0);
    }

    void resize() {
        if (!nodes.empty()) {
            size_t i = nodes.size() - 1;
            for (i = nodes.size() - 1; i > 0; i--) {
                if (nodes.at(i).parent != INVALID_PARENT) {
                    break;
                }
            }
            nodes.resize(i + 1);
        }
    }

    void debug() const {
        std::cout << "debugging octree with width: " << width << std::endl;
        debug(nodes.at(0), Vector3{0.0f}, width / 2, 0);
        std::cout << std::endl;
    }

private:
    static Vector3i align(const Vector3& value) {
        return {
            static_cast<int>(std::floor(value.x)),
            static_cast<int>(std::floor(value.y)),
            static_cast<int>(std::floor(value.z)),
        };
    }

    static size_t idxToMove(const int idx) {
        switch (idx) {
        case 0: {
            return 6;
        }
        case 1: {
            return 7;
        }
        case 2: {
            return 4;
        }
        case 3: {
            return 5;
        }
        case 4: {
            return 2;
        }
        case 5: {
            return 3;
        }
        case 6: {
            return 0;
        }
        case 7: {
            return 1;
        }
        default: {
            throw std::out_of_range("Invalid index");
        }
        }
    }

    static int posToIndex(const Vector3i& pos, const Vector3i& origin) {
        if (pos.y > origin.y) {
            // Top layer
            if (pos.x > origin.x) {
                if (pos.z > origin.z) {
                    return 0;
                } else {
                    return 3;
                }
            } else {
                if (pos.z > origin.z) {
                    return 1;
                } else {
                    return 2;
                }
            }
        } else {
            // Bottom layer
            if (pos.x > origin.x) {
                if (pos.z > origin.z) {
                    return 4;
                } else {
                    return 7;
                }
            } else {
                if (pos.z > origin.z) {
                    return 5;
                } else {
                    return 6;
                }
            }
        }
    }

    void debug(const Node& node, const Vector3i& origin, const int half, const int indent) const {
        for (auto idx = 0; idx < node.children.size(); idx++) {
            const auto childIdx = node.children[idx];
            if (childIdx != 0) {
                const auto offset = idxToOffset(idx, half / 2.0f, Vector3{origin});
                std::cout << std::string(indent, ' ') << "> child at " << offset << std::endl;
                const auto& child = nodes.at(childIdx);
                debug(child, offset, half / 2, indent + 4);
            }
        }
    }

    std::optional<RayCastResut> rayCast(Node& node, const Vector3i& origin, const int half, const Vector3& from,
                                        const Vector3& to) {

        std::optional<RayCastResut> result;

        for (auto idx = 0; idx < node.children.size(); idx++) {
            const auto& childOffset = node.children.at(idx);
            if (childOffset == 0) {
                continue;
            }

            auto& child = nodes.at(childOffset);

            const auto offset = idxToOffset(idx, half / 2.0f, Vector3{origin});

            const auto min = Vector3{offset} - Vector3{static_cast<float>(half) / 2.0f} + Vector3{0.5f};
            const auto max = Vector3{offset} + Vector3{static_cast<float>(half) / 2.0f} + Vector3{0.5f};

            const auto pos = intersectBox(min, max, from, to);
            if (pos.has_value()) {
                if (half == 1) {
                    if (!result.has_value() ||
                        glm::distance(result.value().pos, from) > glm::distance(pos.value(), from)) {
                        result = RayCastResut{child, getNodeOffset(child), pos.value()};
                    }
                } else {
                    const auto test = rayCast(child, offset, half / 2, from, to);
                    if (test.has_value()) {
                        if (!result.has_value() ||
                            glm::distance(result.value().pos, from) > glm::distance(test.value().pos, from)) {
                            result = test;
                        }
                    }
                }
            }
        }

        return result;
    }

    std::optional<NodeRef> find(Node& parent, const Vector3i& origin, const int half, const Vector3i& target) {
        const auto idx = posToIndex(target, origin);
        const auto offset = parent.children[idx];

        if (offset == 0) {
            return std::nullopt;
        }

        auto& child = nodes.at(offset);

        if (half == 1) {
            return NodeRef{child, offset};
        } else {
            return find(child, idxToOffset(idx, half / 2, origin), half / 2, target);
        }
    }

    NodeRef insert(Node& parent, const Vector3i& origin, const int half, const Vector3i& target) {
        // if (half == 1) {

        const auto idx = posToIndex(target, origin);
        auto offset = parent.children[idx];
        Node* child;
        if (offset == 0) {
            const auto parentOffset = getNodeOffset(parent);
            auto& added = nextEmptyNode();
            offset = getNodeOffset(added);
            added.parent = parentOffset;
            child = &added;
            nodes.at(parentOffset).children[idx] = offset;
        } else {
            child = &nodes.at(offset);
        }

        if (half == 1) {
            return {*child, offset};
        } else {
            return insert(*child, idxToOffset(idx, half / 2, origin), half / 2, target);
        }

        /*} else {
            // We need to go deeper
            const auto idx = posToIndex(target, origin);
            const auto offset = parent.children[idx];

        }*/
    }

    void expand(const uint16_t offset) {
        uint16_t grandChildren[8];
        uint16_t children[8] = {0};

        auto& node = nodes.at(offset);

        // const auto s = static_cast<int>(node.size) / 2;
        // const auto origin = node.pos;
        std::memcpy(&grandChildren[0], &node.children[0], sizeof(grandChildren));

        for (auto i = 0; i < 8; i++) {
            if (grandChildren[i] != 0) {
                auto& child = nextEmptyNode();
                auto childIdx = getNodeOffset(child);
                // auto [child, childIdx] = allocator.add();
                children[i] = childIdx;

                // child->size = s << 1;
                // child->dirty = true;
                // child->pos = idxToOffset(i, s, origin);
                child.children[idxToMove(i)] = grandChildren[i];
                child.parent = offset;

                if (grandChildren[i] != 0) {
                    nodes.at(grandChildren[i]).parent = childIdx;
                }
            }
        }

        auto& node0 = nodes.at(offset);
        node0.data = T{};
        std::memcpy(&node0.children[0], &children[0], sizeof(children));
        // node0.size = node0.size << 1;

        width *= 2;
    }

    bool isOutside(const Vector3i& pos) const {
        const auto half = width / 2;
        if (pos.x > half) {
            return true;
        }
        if (pos.y > half) {
            return true;
        }
        if (pos.z > half) {
            return true;
        }
        if (pos.x <= -half) {
            return true;
        }
        if (pos.y <= -half) {
            return true;
        }
        if (pos.z <= -half) {
            return true;
        }
        return false;
    }

    void clearChildOf(Node& parent, const Node& node) {
        const auto offset = getNodeOffset(node);

        auto empty = true;
        for (auto& child : parent.children) {
            if (child == offset) {
                child = 0;
            } else if (child != 0) {
                empty = false;
            }
        }

        if (empty && parent.parent != INVALID_PARENT) {
            clearChildOf(nodes.at(parent.parent), parent);
        }
    }

    template <typename V> static V roundUp(const V numToRound, const V multiple) {
        if (multiple == 0) {
            return numToRound;
        }

        const auto remainder = numToRound % multiple;
        if (remainder == 0) {
            return numToRound;
        }

        return numToRound + multiple - remainder;
    }

    Node& nextEmptyNode() {
        // Check if next empty node
        for (uint32_t i = next; i < static_cast<uint32_t>(nodes.size()); i++) {
            auto& node = nodes.at(i);
            if (node.parent == INVALID_PARENT) {
                next = i + 1;
                return node;
            }
        }

        // No next empty node, expand the array
        const auto last = nodes.size();
        if (nodes.empty()) {
            nodes.resize(1);
        } else {
            auto nextSize = roundUp<size_t>(nodes.size(), 2ULL);
            if (nextSize == nodes.size()) {
                nextSize *= 2;
            }
            if (nextSize > INVALID_PARENT) {
                EXCEPTION("The maximum number of nodes in octree reached");
            }
            nodes.resize(nextSize);
        }
        next = static_cast<uint32_t>(last + 1);
        return nodes.at(last);
    }

    uint16_t getNodeOffset(const Node& node) {
        const auto offset = &node - &nodes.at(0);
        if (offset < 0 || offset >= static_cast<int64_t>(nodes.size())) {
            EXCEPTION("Invalid node and can not calculate the offset");
        }
        return static_cast<uint16_t>(offset);
    }

    std::vector<Node> nodes;
    uint16_t width{0};
    uint32_t next{0};

public:
    MSGPACK_DEFINE_ARRAY(nodes, width);
};
} // namespace Scissio
