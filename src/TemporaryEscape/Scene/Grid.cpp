#include "Grid.hpp"

using namespace Engine;

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

decltype(Grid::ORIENTATIONS) Grid::ORIENTATIONS = {
    glm::rotate(Matrix4{1.0f}, glm::radians(0.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(90.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(270.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(90.0f), Vector3{1.0f, 0.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(270.0f), Vector3{1.0f, 0.0f, 0.0f}),
};

Grid::BlockNode& Grid::insert(const BlockRef& block, const Vector3& pos, const uint8_t rot) {
    return insert(insertBlockType(block), pos, rot);
}

Grid::BlockRef Grid::remove(const BlockNode& node) {
    auto& type = getBlockType(node.data);
    if (type.count > 0) {
        type.count--;
    }
    tree.remove(node);
    type.dirty = true;
    dirty = true;
    return type.block;
}

std::optional<Grid::BlockNodeRef> Grid::find(const Vector3& pos) {
    return tree.find(pos);
}

uint16_t Grid::insertBlockType(const BlockRef& block) {
    for (size_t index = 0; index < types.size(); index++) {
        auto& type = types[index];
        if (type.block.name == block.name) {
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

Grid::BlockNode& Grid::insert(const uint16_t type, const Vector3& pos, const uint8_t rot) {
    const auto ref = tree.insert(pos);
    ref.node.data.type = type;
    ref.node.data.pos = pos;
    ref.node.data.rot = rot;
    dirty = true;
    return ref.node;
}

Grid::BlockType& Grid::getBlockType(const BlockData& node) {
    if (node.type == INVALID_TYPE || node.type >= types.size()) {
        EXCEPTION("Invalid node type");
    }

    auto& type = types.at(node.type);
    if (type.count == 0) {
        EXCEPTION("Invalid node type");
    }

    return type;
}

const Grid::BlockType& Grid::getBlockType(const BlockData& node) const {
    if (node.type == INVALID_TYPE || node.type >= types.size()) {
        EXCEPTION("Invalid node type");
    }

    auto& type = types.at(node.type);
    if (type.count == 0) {
        EXCEPTION("Invalid node type");
    }

    return type;
}

std::vector<Grid::BlockInstances> Grid::buildInstanceBuffer() {
    std::unordered_map<uint16_t, std::vector<Matrix4>> matrices;

    for (uint16_t typeIdx = 0; typeIdx < static_cast<uint16_t>(types.size()); typeIdx++) {
        auto& type = types.at(typeIdx);

        if (!type.block.model) {
            continue;
        }

        if (type.dirty) {
            matrices.insert(std::make_pair(typeIdx, std::vector<Matrix4>{}));
        }
        type.dirty = false;
    }

    for (const auto& node : tree.getNodes()) {
        if (node.data.type == INVALID_TYPE) {
            continue;
        }

        if (auto it = matrices.find(node.data.type); it != matrices.end()) {
            Matrix4 transformation{1.0f};
            auto rot = node.data.rot;
            if (rot < 0 || rot >= ORIENTATIONS.size()) {
                rot = 0;
            }
            transformation = glm::translate(transformation, node.data.pos);
            transformation *= ORIENTATIONS[rot];
            it->second.push_back(transformation);
        }
    }

    std::vector<BlockInstances> blocks;

    for (auto& pair : matrices) {
        const auto& type = types.at(pair.first);
        blocks.emplace_back();
        blocks.back().model = type.block.model;
        blocks.back().instances = std::move(pair.second);
        blocks.back().type = pair.first;
    }

    dirty = false;
    return blocks;
}

std::optional<Grid::RayCastResult> Grid::rayCast(const Vector3& from, const Vector3& to) {
    auto res = tree.rayCast(from, to);
    if (!res.has_value()) {
        return std::nullopt;
    }

    auto& node = res.value().node.get();
    auto& data = node.data;
    const auto block = getBlockType(data).block;

    const auto normal = intersectBoxNormal(data.pos, res.value().pos);

    auto side = 0;
    if (normal.x <= -1.0f) {
        side = 1;
    }
    if (normal.y >= 1.0f) {
        side = 2;
    }
    if (normal.y <= -1.0f) {
        side = 3;
    }
    if (normal.z >= 1.0f) {
        side = 4;
    }
    if (normal.z <= -1.0f) {
        side = 5;
    }

    return RayCastResult{node, block, res.value().pos, normal, side};
}

std::optional<Grid::RayCastResult> Grid::rayCast(const Vector3& from, const Vector3& to, const Matrix4& transform) {
    const auto inverse = glm::inverse(transform);
    const auto alignedFrom = Vector3{inverse * Vector4{from, 1.0f}};
    const auto alignedTo = Vector3{inverse * Vector4{to, 1.0f}};

    const auto result = rayCast(alignedFrom, alignedTo);
    if (!result.has_value()) {
        return std::nullopt;
    }

    return RayCastResult{
        result.value().node,   result.value().block, Vector3{transform * Vector4{result.value().pos, 1.0f}},
        result.value().normal, result.value().side,
    };
}
