#pragma once

#include "../Assets/Block.hpp"
#include "../Assets/VoxelShapeCache.hpp"
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Bitfield.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/MsgpackAdaptors.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <vector>

namespace Engine {
class ENGINE_API Grid {
public:
    template <typename T, size_t MaxSize, size_t BucketSize = 1024> class Pool {
    public:
        static constexpr size_t maxSize = MaxSize;
        static constexpr size_t bucketSize = BucketSize;

        [[nodiscard]] T* data() {
            return items.data();
        }

        [[nodiscard]] T& at(size_t idx) {
            if (idx > items.size()) {
                EXCEPTION("Out of bounds");
            }
            return items.at(idx);
        }

        [[nodiscard]] const T& at(size_t idx) const {
            if (idx > items.size()) {
                EXCEPTION("Out of bounds");
            }
            return items.at(idx);
        }

        [[nodiscard]] size_t indexOf(const T& item) const {
            const auto idx = &item - items.data();
            if (idx >= items.size()) {
                EXCEPTION("Bad indexOf item value");
            }
            return idx;
        }

        void erase(size_t idx) {
            if (idx > items.size()) {
                EXCEPTION("Out of bounds");
            }

            auto& item = items.at(idx);
            if (item) {
                auto& counter = counters.at(idx / bucketSize);
                if (counter == 0) {
                    EXCEPTION("Malformed pool counters during decrement");
                }
                counter--;
            }
        }

        T& insert() {
            auto emptyIdx = nextEmpty();
            if (emptyIdx == maxSize) {
                if (counters.size() == maxSize / bucketSize) {
                    EXCEPTION("Maximum size reached");
                }

                emptyIdx = items.size();
                items.resize(items.size() + bucketSize);
                std::memset(items.data() + emptyIdx, 0x00, bucketSize * sizeof(T));
                counters.push_back(0);
            }

            auto& counter = counters.at(emptyIdx / bucketSize);
            if (counter >= bucketSize) {
                EXCEPTION("Malformed pool counters during increment");
            }
            counter++;
            // Log::d("Grid", "Inserting new node at: {}", emptyIdx);
            return items.at(emptyIdx);
        }

        [[nodiscard]] size_t nextEmpty() const {
            for (size_t c = 0; c < counters.size(); c++) {
                const auto& counter = counters.at(c);
                if (c * bucketSize + bucketSize > items.size()) {
                    EXCEPTION("Malformed pool data");
                }

                if (counter < bucketSize) {
                    for (size_t i = c * bucketSize; i < c * bucketSize + bucketSize; i++) {
                        if (!items.at(i) && i != 0) {
                            return i;
                        }
                    }
                }
            }

            return maxSize;
        }

        [[nodiscard]] size_t size() const {
            size_t s = 0;
            for (const auto& c : counters) {
                s += c;
            }
            return s;
        }

        void convert(msgpack::object const& o) {
            if (o.type != msgpack::type::ARRAY || o.via.array.size != 2) {
                throw msgpack::type_error();
            }

            auto& e0 = o.via.array.ptr[0];
            if (e0.type != msgpack::type::BIN) {
                throw msgpack::type_error();
            }

            auto& e1 = o.via.array.ptr[1];
            if (e1.type != msgpack::type::BIN) {
                throw msgpack::type_error();
            }

            items.resize(e0.via.bin.size / sizeof(T));
            std::memcpy(items.data(), e0.via.bin.ptr, e0.via.bin.size);

            counters.resize(e1.via.bin.size / sizeof(uint16_t));
            std::memcpy(counters.data(), e1.via.bin.ptr, e1.via.bin.size);
        }

        template <typename Stream> void pack(msgpack::packer<Stream>& o) const {
            o.pack_array(2);
            o.pack_bin(items.size() * sizeof(T));
            o.pack_bin_body(reinterpret_cast<const char*>(items.data()), items.size() * sizeof(T));
            o.pack_bin(counters.size() * sizeof(uint16_t));
            o.pack_bin_body(reinterpret_cast<const char*>(counters.data()), counters.size() * sizeof(uint16_t));
        }

    private:
        std::vector<T> items;
        std::vector<uint16_t> counters;
    };

    using Index = uint32_t;

    union Voxel {
        uint64_t data;

        Bitfield<Index, 0, 24> next;
        Bitfield<uint8_t, 24, 8> color;
        Bitfield<uint16_t, 32, 10> type;
        Bitfield<uint8_t, 42, 5> rotation;
        Bitfield<uint8_t, 47, 3> index;
        Bitfield<uint8_t, 50, 5> shape;

        operator bool() const {
            return bool(data);
        }

        Voxel& operator=(const Voxel& other) {
            data = other.data;
            return *this;
        }

        [[nodiscard]] std::string string() const {
            return fmt::format("<index: {}, next: {}, color: {}, type: {}, rotation: {} shape: {}>",
                               int(index),
                               int(next),
                               int(color),
                               int(type),
                               int(rotation),
                               int(shape));
        }
    };

    union Branch {
        uint64_t data;

        Bitfield<Index, 0, 24> next;
        Bitfield<Index, 24, 24> child;
        Bitfield<bool, 48, 1> compressed;
        Bitfield<uint8_t, 49, 3> index;

        operator bool() const {
            return bool(data);
        }

        Branch& operator=(const Branch& other) {
            data = other.data;
            return *this;
        }

        [[nodiscard]] std::string string() const {
            return fmt::format("<index: {}, next: {}, child: {}, compressed: {}>",
                               int(index),
                               int(next),
                               int(child),
                               bool(compressed));
        }
    };

    union Node {
        Voxel voxel;
        Branch branch;
        uint64_t data;

        operator bool() const {
            return bool(data);
        }

        Node& operator=(const Node& other) {
            data = other.data;
            return *this;
        }
    };

    struct Type {
        BlockPtr block{nullptr};
        size_t count{0};

        MSGPACK_DEFINE_ARRAY(block, count);
    };

    struct ThrusterInfo {
        Matrix4 mat;
        ParticlesTypePtr particles;
    };

    struct BlocksData {
        std::vector<VoxelShape::VertexFinal> vertices;
        std::vector<uint32_t> indices;
        std::vector<ThrusterInfo> thrusters;
    };

    static constexpr size_t maxNodesPerTree = 256 * 256 * 256;
    static constexpr size_t badIndex = 0xFFFFFF;
    static constexpr size_t meshBuildWidth = 16;
    static constexpr size_t cacheBuildWidth = meshBuildWidth + 2;

    using NodesPool = Pool<Node, maxNodesPerTree>;

    static inline int getWidthForLevel(const size_t level) {
        return static_cast<int>(std::pow(2, level)) / 2;
    }

    struct RayCastResult {
        std::reference_wrapper<const Node> node;
        Vector3 normal;
        Vector3 hitPos;
        Vector3i pos;
        Vector3 worldPos;
        Vector3i orientation;
    };

    class ENGINE_API Octree;

    class ENGINE_API Iterator {
    public:
        Iterator() = default;
        explicit Iterator(Octree& octree, size_t idx, size_t level, const Vector3i& origin);
        Iterator(const Iterator& other) = default;
        Iterator& operator=(const Iterator& other) = default;

        Node& value();
        void next();
        [[nodiscard]] Iterator children();

        operator bool() const {
            return idx != badIndex;
        }

        [[nodiscard]] size_t getLevel() const {
            return level;
        }

        [[nodiscard]] bool isVoxel() const;
        [[nodiscard]] int getBranchWidth() const;
        [[nodiscard]] const Vector3i& getOrigin() const {
            return origin;
        }
        [[nodiscard]] const Vector3i& getPos() const {
            return pos;
        }

    private:
        Octree* octree{nullptr};
        size_t idx{badIndex};
        size_t level{0};
        Vector3i origin{0};
        Vector3i pos{0};
    };

    class ENGINE_API Octree {
    public:
        Octree();
        void insert(const Vector3i& pos, const Voxel& voxel);
        bool remove(const Vector3i& pos);

        void expand(Index idx, size_t level);

        [[nodiscard]] bool isOutside(const Vector3i& pos) const;
        [[nodiscard]] int getWidth() const;

        [[nodiscard]] size_t getDepth() const {
            return depth;
        }

        [[nodiscard]] std::optional<Voxel> find(const Vector3i& pos) const;

        [[nodiscard]] std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to) const;

        [[nodiscard]] size_t size() const {
            return nodes.size();
        }

        void dump() const;

        NodesPool& pool() {
            return nodes;
        }

        [[nodiscard]] const NodesPool& pool() const {
            return nodes;
        }

        Iterator iterate() {
            return Iterator(*this, 0, 0, Vector3i{0});
        }

        MSGPACK_DEFINE_ARRAY(nodes, depth);

    private:
        void insert(Node& parent, const Vector3i& origin, const Vector3i& pos, const Voxel& voxel, size_t level);
        bool remove(Node& parent, const Vector3i& origin, const Vector3i& pos, size_t level);
        [[nodiscard]] std::optional<Voxel> find(const Node& parent, const Vector3i& origin, const Vector3i& pos,
                                                size_t level) const;
        [[nodiscard]] std::optional<RayCastResult> rayCast(const Node& parent, const Vector3i& origin, size_t level,
                                                           const Vector3& from, const Vector3& to) const;
        void dump(const Node& node, size_t level = 0) const;

        NodesPool nodes;
        size_t depth{0};
    };

    void insert(const Vector3i& pos, const BlockPtr& block, uint8_t rotation, uint8_t color, uint8_t shape);
    void insert(const Vector3i& pos, const uint16_t type, uint8_t rotation, uint8_t color, uint8_t shape);
    bool remove(const Vector3i& pos);
    void updateBounds();

    [[nodiscard]] std::optional<Voxel> find(const Vector3i& pos) const {
        return voxels.find(pos);
    }

    void generateMesh(const VoxelShapeCache& voxelShapeCache, BlocksData& data);

    [[nodiscard]] std::optional<RayCastResult> rayCast(const Vector3& from, const Vector3& to) const;

    void dump() {
        voxels.dump();
    }

    Iterator iterate() {
        return voxels.iterate();
    }

    [[nodiscard]] const NodesPool& pool() const {
        return voxels.pool();
    }

    [[nodiscard]] const BlockPtr& getType(const size_t index) const;

    std::optional<uint16_t> getTypeIndex(const BlockPtr& block) const;
    size_t getTypeCount(const size_t index) const;

    size_t getTypeCount() const {
        return types.size();
    }

    float getRadius() const {
        return bbRadius;
    }

    MSGPACK_DEFINE_ARRAY(voxels, types);

private:
    uint16_t insertBlock(const BlockPtr& block);
    void generateMesh(Iterator& iterator, const VoxelShapeCache& voxelShapeCache, BlocksData& data);
    void generateMeshBlock(Iterator& iterator, const VoxelShapeCache& voxelShapeCache, BlocksData& data);
    void generateMeshCache(Iterator& iterator, Voxel* cache, const Vector3i& offset) const;
    void build(const VoxelShapeCache& voxelShapeCache, const Voxel* cache, const std::vector<Type>& types,
               BlocksData& data, const Vector3i& offset);
    void updateBounds(Iterator& iterator);
    // void buildBlock(const Voxel& voxel, BlockBuilder& blockBuilder, const Vector3i& pos, TypePrimitiveMap& map);

    Octree voxels;
    std::vector<Type> types;
    float bbRadius;
};

inline bool Grid::Iterator::isVoxel() const {
    return octree->getDepth() - level == 0;
}

inline Grid::Node& Grid::Iterator::value() {
    return octree->pool().at(idx);
}

inline int Grid::Iterator::getBranchWidth() const {
    return Grid::getWidthForLevel(octree->getDepth() - level + 1);
}
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <typename T, size_t MaxSize, size_t BucketSize>
    struct convert<Engine::Grid::Pool<T, MaxSize, BucketSize>> {
        msgpack::object const& operator()(msgpack::object const& o,
                                          Engine::Grid::Pool<T, MaxSize, BucketSize>& v) const {
            v.convert(o);
            return o;
        }
    };

    template <typename T, size_t MaxSize, size_t BucketSize> struct pack<Engine::Grid::Pool<T, MaxSize, BucketSize>> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o,
                                            Engine::Grid::Pool<T, MaxSize, BucketSize> const& v) const {
            v.pack(o);
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
