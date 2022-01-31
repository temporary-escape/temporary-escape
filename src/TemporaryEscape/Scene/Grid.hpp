#pragma once
#include "../Assets/AssetModel.hpp"
#include "Octree.hpp"

namespace Engine {
class ENGINE_API Grid {
public:
    static constexpr uint16_t INVALID_TYPE = 0xFFFF;

    static const std::array<Matrix4, 6> ORIENTATIONS;

    struct BlockRef {
        BlockRef() = default;

        explicit BlockRef(AssetModelPtr model) : name(model->getName()), model(std::move(model)) {
        }

        explicit BlockRef(std::string name, AssetModelPtr model = nullptr)
            : name(std::move(name)), model(std::move(model)) {
        }

        std::string name;
        AssetModelPtr model{nullptr};

        MSGPACK_DEFINE_ARRAY(name, model);
    };

    struct BlockType {
        BlockRef block;
        uint16_t count{0};
        bool dirty{true};

        MSGPACK_DEFINE_ARRAY(block, count, dirty);
    };

    using Types = std::vector<BlockType>;

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
        BlockRef block;
        Vector3 pos{0.0};
        Vector3 normal{0.0f};
        int side{0};
    };

    struct BlockInstances {
        uint16_t type{0};
        AssetModelPtr model;
        std::vector<Matrix4> instances;
    };

    Grid() = default;
    virtual ~Grid() = default;

    BlockNode& insert(const BlockRef& block, const Vector3& pos, uint8_t rot);
    BlockRef remove(const BlockNode& node);
    std::optional<BlockNodeRef> find(const Vector3& pos);

    BlockType& getBlockType(const BlockData& node);
    const BlockType& getBlockType(const BlockData& node) const;

    const Types& getTypes() const {
        return types;
    }

    const std::vector<Octree<BlockData>::Node>& getNodes() const {
        return tree.getNodes();
    }

    bool isDirty() const {
        return dirty;
    }

    void clearDirty() {
        dirty = false;
    }

    void debugTree() {
        tree.debug();
    }

    std::vector<BlockInstances> buildInstanceBuffer();

    std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to);
    std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to, const Matrix4& transform);

private:
    uint16_t insertBlockType(const BlockRef& block);
    BlockNode& insert(uint16_t type, const Vector3& pos, uint8_t rot);

    Octree<BlockData> tree;
    Types types;
    // Nodes nodes;
    size_t next{0};
    bool dirty{true};

public:
    MSGPACK_DEFINE_ARRAY(tree, types);
};
} // namespace Engine
