#include "ComponentGrid.hpp"
#include "../Shaders/ShaderGrid.hpp"

using namespace Engine;

decltype(ComponentGrid::ORIENTATIONS) ComponentGrid::ORIENTATIONS = {
    glm::rotate(Matrix4{1.0f}, glm::radians(0.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(90.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(270.0f), Vector3{0.0f, 1.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(90.0f), Vector3{1.0f, 0.0f, 0.0f}),
    glm::rotate(Matrix4{1.0f}, glm::radians(270.0f), Vector3{1.0f, 0.0f, 0.0f}),
};

ComponentGrid::BlockNode& ComponentGrid::insert(const AssetBlockPtr& block, const Vector3i& pos, const uint8_t rot) {
    return insert(insertBlockType(block), Vector3{pos}, rot);
}

AssetBlockPtr ComponentGrid::remove(const BlockNode& node) {
    auto& type = getBlockType(node.data);
    if (type.count > 0) {
        type.count--;
    }
    tree.remove(node);
    type.dirty = true;
    shouldRebuild = true;
    return type.block;
}

std::optional<ComponentGrid::BlockNodeRef> ComponentGrid::find(const Vector3& pos) {
    return tree.find(pos);
}

uint16_t ComponentGrid::insertBlockType(const AssetBlockPtr& block) {
    for (size_t index = 0; index < types.size(); index++) {
        auto& type = types[index];
        if (type.block->getName() == block->getName()) {
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

ComponentGrid::BlockNode& ComponentGrid::insert(const uint16_t type, const Vector3& pos, const uint8_t rot) {
    const auto ref = tree.insert(pos);
    ref.node.data.type = type;
    ref.node.data.pos = pos;
    ref.node.data.rot = rot;
    shouldRebuild = true;
    return ref.node;
}

ComponentGrid::BlockType& ComponentGrid::getBlockType(const BlockData& node) {
    if (node.type == INVALID_TYPE || node.type >= types.size()) {
        EXCEPTION("Invalid node type");
    }

    auto& type = types.at(node.type);
    if (type.count == 0) {
        EXCEPTION("Invalid node type");
    }

    return type;
}

const ComponentGrid::BlockType& ComponentGrid::getBlockType(const BlockData& node) const {
    if (node.type == INVALID_TYPE || node.type >= types.size()) {
        EXCEPTION("Invalid node type");
    }

    auto& type = types.at(node.type);
    if (type.count == 0) {
        EXCEPTION("Invalid node type");
    }

    return type;
}

std::vector<ComponentGrid::BlockTypeMesh> ComponentGrid::generateMeshes() {
    std::unordered_map<uint16_t, std::vector<Matrix4>> matricesMap;

    for (uint16_t typeIdx = 0; typeIdx < static_cast<uint16_t>(types.size()); typeIdx++) {
        auto& type = types.at(typeIdx);

        if (!type.block->getModel()) {
            continue;
        }

        if (type.dirty) {
            std::vector<Matrix4> matrices;
            matrices.reserve(type.count);
            matricesMap.insert(std::make_pair(typeIdx, std::vector<Matrix4>{}));
        }
        type.dirty = false;
    }

    for (const auto& node : tree.getNodes()) {
        if (node.data.type == INVALID_TYPE) {
            continue;
        }

        if (auto it = matricesMap.find(node.data.type); it != matricesMap.end()) {
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

    std::vector<BlockTypeMesh> results;

    for (const auto& [typeIdx, matrices] : matricesMap) {
        if (typeIdx > types.size()) {
            continue;
        }

        const auto& type = types.at(typeIdx);

        results.emplace_back();
        auto& result = results.back();

        result.block = type.block;

        result.instances = VertexBuffer(VertexBufferType::Array);
        result.instances.bufferData(matrices.data(), matrices.size() * sizeof(Matrix4), VertexBufferUsage::StaticDraw);

        auto& primitives = type.block->getModel()->getPrimitives();
        for (auto& primitive : primitives) {
            result.primitives.emplace_back();
            auto& p = result.primitives.back();

            p.ubo = const_cast<VertexBuffer*>(&primitive.ubo);
            p.material = const_cast<Material*>(&primitive.material);

            p.mesh = Mesh{};
            p.mesh.addVertexBuffer(primitive.vbo, ShaderGrid::Position{}, ShaderGrid::Normal{},
                                   ShaderGrid::TextureCoordinates{}, ShaderGrid::Tangent{});
            p.mesh.addVertexBufferInstanced(result.instances, ShaderGrid::Instances{});
            p.mesh.setIndexBuffer(primitive.ibo, primitive.mesh.getIndexType());
            p.mesh.setPrimitive(primitive.mesh.getPrimitive());
            p.mesh.setCount(primitive.mesh.getCount());
            p.mesh.setInstancesCount(static_cast<GLsizei>(matrices.size()));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }

    return results;
}

std::optional<ComponentGrid::RayCastResult> ComponentGrid::rayCast(const Vector3& from, const Vector3& to) {
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

std::optional<ComponentGrid::RayCastResult> ComponentGrid::rayCast(const Vector3& from, const Vector3& to,
                                                                   const Matrix4& transform) {
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
