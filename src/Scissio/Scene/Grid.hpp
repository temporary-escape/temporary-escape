#pragma once
#include "../Assets/Block.hpp"
#include "Octree.hpp"

namespace Scissio {
class SCISSIO_API Grid {
public:
    static constexpr uint16_t INVALID_TYPE = 0xFFFF;

    using Pointer = uint16_t;

    struct NodeType {
        BlockPtr block{nullptr};
        uint16_t count{0};
        bool dirty{true};

        MSGPACK_DEFINE_ARRAY(block, count, dirty);
    };

    using Types = std::vector<NodeType>;

    struct Node {
        uint16_t type{INVALID_TYPE};
        Vector3 pos{0};
        uint8_t rot{0};

        MSGPACK_DEFINE_ARRAY(type, pos, rot);
    };

    struct RayCastResult {
        std::reference_wrapper<Node> node;
        BlockPtr block{nullptr};
        Vector3 pos{0.0};
        Vector3 normal{0.0f};
        int side{0};
    };

    using Nodes = std::vector<Node>;

    Grid() = default;
    virtual ~Grid() = default;

    Node& insert(const BlockPtr& block, const Vector3& pos, uint8_t rot);
    NodeType& getType(const Node& node);
    const NodeType& getType(const Node& node) const;

    const Types& getTypes() const {
        return types;
    }

    const Nodes& getNodes() const {
        return nodes;
    }

    void resize();

    bool isDirtyClear() {
        if (dirty) {
            dirty = false;
            return true;
        }
        return false;
    }

    std::unordered_map<BlockPtr, std::vector<Matrix4>> buildInstanceBuffer();

    std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to);
    std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to, const Matrix4& transform);

private:
    uint16_t insertBlockType(const BlockPtr& block);
    Node& insert(uint16_t type, const Vector3& pos, uint8_t rot);
    Node& nextEmptyNode();

    Octree<Pointer> tree;
    Types types;
    Nodes nodes;
    size_t next{0};
    bool dirty{true};

public:
    MSGPACK_DEFINE_ARRAY(nodes, types, nodes);
};
} // namespace Scissio
