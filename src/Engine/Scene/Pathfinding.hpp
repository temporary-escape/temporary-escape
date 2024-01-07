#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include <array>
#include <functional>
#include <queue>
#include <vector>

namespace Engine {
class ENGINE_API Pathfinding {
public:
    class Tester {
    public:
        virtual ~Tester() = default;

        virtual bool contactTestBox(const Vector3i& pos, int width) = 0;
    };

    using Index = uint32_t;
    using ChildrenMask = uint8_t;
    using HashCode = uint64_t;

    struct Node {
        Index offset;
        ChildrenMask children;
    };

    struct NodeInfo {
        Vector3i pos;
        Index offset;
        HashCode code;
        int level;

        operator bool() const {
            return offset != 0;
        }
    };

    using NodeInfoCallback = std::function<void(const NodeInfo&)>;

    struct NodeInfoDistance {
        NodeInfo info;
        uint64_t distance;

        bool operator<(const NodeInfoDistance& other) const {
            return distance < other.distance;
        }
    };

    struct Info {
        HashCode from;
        uint64_t cost;
    };

    struct QueryData {
        NodeInfo start;
        NodeInfo goal;
        std::priority_queue<NodeInfoDistance> frontier;
        std::unordered_map<HashCode, Info> costSoFar;
        NodeInfoDistance current;
    };

    explicit Pathfinding(Tester& tester, int depth, int scale);

    //using Node = uint64_t;

    //size_t addNode();

    void build();
    std::optional<HashCode> find(const Vector3& pos);
    NodeInfo findNearest(const Vector3& pos, int maxLevel = std::numeric_limits<int>::max());
    bool findPath(const Vector3& from, const Vector3& to);
    //void iterate(const std::function<void(const Vector3i&, int)>& fn);

    [[nodiscard]] int getDepth() const {
        return depth;
    }

    [[nodiscard]] int getScale() const {
        return scale;
    }

private:
    using Nodes = std::vector<Node>;
    using Layers = std::vector<Nodes>;

    static constexpr size_t nodeCapacityMultipler{1024};

    /*struct Bucket {
        size_t used{0};
        size_t next{0};
    };*/

    Index allocateTempNodes(int level);
    Index allocateNodes(size_t num);
    void optimize();
    void build(Index index, const Vector3i& origin, int level);
    void getNeighbours(const NodeInfo& info, QueryData& data);
    void getNeighboursSide(const Index index, const Vector3i& origin, int level, HashCode previous, uint8_t side, QueryData& data);
    void processNeighbour(const NodeInfo& next, QueryData& data);
    std::optional<HashCode> find(Index index, const Vector3i& origin, int level, HashCode previous, const Vector3& pos);
    NodeInfo findNearest(Index index, const Vector3i& origin, int level, HashCode previous, const Vector3& pos, int maxLevel);

    //void iterate(size_t node, const Vector3i& origin, int level, const std::function<void(const Vector3i&, int)>& fn);

    //inline void setNodeValue(size_t node, uint64_t offset, uint64_t mask, uint64_t value);
    //inline uint64_t getNodeValue(size_t node, uint64_t offset, uint64_t mask);

    Tester& tester;
    const int depth;
    const int scale;
    //std::vector<Bucket> buckets;

    std::vector<Node> nodes;
    //std::vector<Info> infos;
    Layers temp;
    std::array<size_t, 16> levels;
};
} // namespace Engine
