#pragma once

#include "../Assets/AssetBlock.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "Octree.hpp"

namespace Engine {
class ENGINE_API ComponentGrid : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    struct BlockPrimitive {
        Mesh mesh{NO_CREATE};
        VertexBuffer* ubo{nullptr};
        Material* material{nullptr};
    };

    struct BlockType {
        AssetBlockPtr block;
        uint16_t count{0};
        bool dirty{true};

        MSGPACK_DEFINE_ARRAY(block, count, dirty);
    };

    struct BlockTypeMesh {
        AssetBlockPtr block;
        VertexBuffer instances{NO_CREATE};
        std::list<BlockPrimitive> primitives;
    };

    struct BlockData {
        uint16_t type{INVALID_TYPE};
        Vector3 pos{0};
        uint8_t rot{0};

        MSGPACK_DEFINE_ARRAY(type, pos, rot);
    };

    using BlockNode = Octree<BlockData>::Node;
    using BlockNodeRef = Octree<BlockData>::NodeRef;

    struct RayCastResult {
        std::reference_wrapper<BlockNode> node;
        AssetBlockPtr block;
        Vector3 pos{0.0};
        Vector3 normal{0.0f};
        int side{0};
    };

    static constexpr uint16_t INVALID_TYPE = 0xFFFF;
    static const std::array<Matrix4, 6> ORIENTATIONS;

    using Types = std::vector<BlockType>;
    using BlockTypeMeshMap = std::unordered_map<AssetBlockPtr, BlockTypeMesh>;

    ComponentGrid() = default;

    explicit ComponentGrid(Object& object) : Component(object) {
    }
    virtual ~ComponentGrid() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    BlockNode& insert(const AssetBlockPtr& block, const Vector3i& pos, uint8_t rot);
    AssetBlockPtr remove(const BlockNode& node);
    std::optional<BlockNodeRef> find(const Vector3& pos);

    BlockType& getBlockType(const BlockData& node);
    const BlockType& getBlockType(const BlockData& node) const;

    const Types& getTypes() const {
        return types;
    }

    const std::vector<Octree<BlockData>::Node>& getNodes() const {
        return tree.getNodes();
    }

    void debugTree() {
        tree.debug();
    }

    std::vector<BlockTypeMesh> generateMeshes();

    std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to);
    std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to, const Matrix4& transform);

    void rebuild() {
        if (!shouldRebuild) {
            return;
        }

        rebuildMeshes();
        shouldRebuild = false;
    }

    void rebuildMeshes() {
        auto results = generateMeshes();
        for (auto& result : results) {
            meshes.insert(std::make_pair(result.block, std::move(result)));
        }
    }

    const BlockTypeMeshMap& getMeshes() const {
        return meshes;
    }

private:
    BlockTypeMeshMap meshes;

    uint16_t insertBlockType(const AssetBlockPtr& block);
    BlockNode& insert(uint16_t type, const Vector3& pos, uint8_t rot);

    Octree<BlockData> tree;
    Types types;
    size_t next{0};
    bool shouldRebuild{true};

public:
    MSGPACK_DEFINE_ARRAY(tree, types);
};
} // namespace Engine
