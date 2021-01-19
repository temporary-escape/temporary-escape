#include "Grid.hpp"

using namespace Scissio;

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

Grid::Node& Grid::insert(const BlockPtr& block, const Vector3& pos, const uint8_t rot) {
    return insert(insertBlockType(block), pos, rot);
}

uint16_t Grid::insertBlockType(const BlockPtr& block) {
    for (size_t index = 0; index < types.size(); index++) {
        auto& type = types[index];
        if (type.block == block) {
            type.count++;
            type.dirty = true;
            return static_cast<uint16_t>(index);
        }
    }

    if (types.size() >= INVALID_TYPE) {
        EXCEPTION("Can not insert any more block types");
    }

    types.push_back({});
    auto& type = types.back();
    type.block = block;
    type.count = 1;
    type.dirty = true;

    return static_cast<uint16_t>(types.size() - 1);
}

Grid::Node& Grid::insert(const uint16_t type, const Vector3& pos, const uint8_t rot) {
    auto& node = nextEmptyNode();
    const auto ref = tree.insert(pos);
    ref.node.data = static_cast<Pointer>(&nodes.at(0) - &node);
    node.type = type;
    node.pos = pos;
    node.rot = rot;
    dirty = true;
    return node;
}

Grid::Node& Grid::nextEmptyNode() {
    // Check if next empty node
    for (size_t i = next; i < nodes.size(); i++) {
        auto& node = nodes.at(i);
        if (node.type == INVALID_TYPE) {
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
        if (nextSize > INVALID_TYPE) {
            EXCEPTION("The maximum number of nodes in grid reached");
        }
        nodes.resize(nextSize);
    }
    next = last + 1;
    return nodes.at(last);
}

Grid::NodeType& Grid::getType(const Node& node) {
    if (node.type == INVALID_TYPE || node.type >= types.size()) {
        EXCEPTION("Invalid node type");
    }

    auto& type = types.at(node.type);
    if (type.count == 0) {
        EXCEPTION("Invalid node type");
    }

    return type;
}

const Grid::NodeType& Grid::getType(const Node& node) const {
    if (node.type == INVALID_TYPE || node.type >= types.size()) {
        EXCEPTION("Invalid node type");
    }

    auto& type = types.at(node.type);
    if (type.count == 0) {
        EXCEPTION("Invalid node type");
    }

    return type;
}

void Grid::resize() {
    if (!nodes.empty()) {
        size_t i = nodes.size() - 1;
        for (i = nodes.size() - 1; i > 0; i--) {
            if (nodes.at(i).type != INVALID_TYPE) {
                break;
            }
        }
        nodes.resize(i + 1);
    }

    tree.resize();
}

std::unordered_map<BlockPtr, std::vector<Matrix4>> Grid::buildInstanceBuffer() {
    std::unordered_map<uint16_t, std::vector<Matrix4>> matrices;

    for (uint16_t typeIdx = 0; typeIdx < static_cast<uint16_t>(types.size()); typeIdx++) {
        auto& type = types.at(typeIdx);

        if (!type.block) {
            continue;
        }

        if (type.dirty) {
            matrices.insert(std::make_pair(typeIdx, std::vector<Matrix4>{}));
        }
        type.dirty = false;
    }

    for (const auto& node : nodes) {
        if (auto it = matrices.find(node.type); it != matrices.end()) {
            Matrix4 transformation{1.0f};
            transformation = glm::translate(transformation, node.pos);
            it->second.push_back(transformation);
        }
    }

    std::unordered_map<BlockPtr, std::vector<Matrix4>> blocks;

    for (auto& pair : matrices) {
        const auto& type = types.at(pair.first);
        blocks.insert(std::make_pair(type.block, std::move(pair.second)));
    }

    dirty = false;
    return blocks;
}

std::optional<Grid::RayCastResult> Grid::rayCast(const Vector3& from, const Vector3& to) {
    auto res = tree.rayCast(from, to);
    if (!res.has_value()) {
        return std::nullopt;
    }

    auto& node = nodes.at(res.value().node.data);
    const auto block = getType(node).block;

    return RayCastResult{node, block, res.value().pos};
}

std::optional<Grid::RayCastResult> Grid::rayCast(const Vector3& from, const Vector3& to, const Matrix4& transform) {
    const auto inverse = glm::inverse(transform);
    const auto alignedFrom = Vector3{inverse * Vector4{from, 1.0f}};
    const auto alignedTo = Vector3{inverse * Vector4{to, 1.0f}};

    return rayCast(alignedFrom, alignedTo);
}
