#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include <array>
#include <functional>
#include <vector>

namespace Engine {
class ENGINE_API Pathfinding {
public:
    class Tester {
    public:
        virtual ~Tester() = default;

        virtual bool contactTestBox(const Vector3& pos, float width) = 0;
    };

    Pathfinding(Tester& tester, int depth, int scale);

    using Node = uint64_t;

    size_t addNode();

    void build();
    size_t find(const Vector3& pos);
    void iterate(const std::function<void(const Vector3i&, int)>& fn);

    [[nodiscard]] int getDepth() const {
        return depth;
    }

    [[nodiscard]] int getScale() const {
        return scale;
    }

    [[nodiscard]] size_t getCount() const {
        return count;
    }

private:
    static constexpr size_t bucketSize{1024};

    struct Bucket {
        size_t used{0};
        size_t next{0};
    };

    void build(size_t node, const Vector3i& origin, int level);
    size_t find(size_t node, const Vector3i& origin, int level, const Vector3& pos);
    void iterate(size_t node, const Vector3i& origin, int level, const std::function<void(const Vector3i&, int)>& fn);

    inline void setNodeValue(size_t node, uint64_t offset, uint64_t mask, uint64_t value);
    inline uint64_t getNodeValue(size_t node, uint64_t offset, uint64_t mask);

    Tester& tester;
    const int depth;
    const int scale;
    size_t count;
    std::vector<Bucket> buckets;
    std::vector<Node> nodes;
};
} // namespace Engine
