#pragma once

#include "../Library.hpp"
#include "../Math/Utils.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Bitfield.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Msgpack.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <vector>

namespace Engine {
class Grid {
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

        operator bool() const {
            return bool(data);
        }

        Voxel& operator=(const Voxel& other) {
            data = other.data;
            return *this;
        }

        [[nodiscard]] std::string string() const {
            return fmt::format("<index: {}, next: {}, color: {}, type: {}, rotation: {}>", int(index), int(next),
                               int(color), int(type), int(rotation));
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
            return fmt::format("<index: {}, next: {}, child: {}, compressed: {}>", int(index), int(next), int(child),
                               bool(compressed));
        }
    };

    union Node {
        Node() = default;

        Voxel voxel;
        Branch branch;
        uint64_t data{0};

        operator bool() const {
            return bool(data);
        }

        Node& operator=(const Node& other) {
            data = other.data;
            return *this;
        }
    };

    static constexpr size_t maxNodesPerTree = 256 * 256 * 256;
    static constexpr size_t badIndex = 0xFFFFFF;

    class Octree {
    public:
        Octree();
        void insert(const Vector3i& pos, const Voxel& voxel);

        void expand(Index idx, size_t level);

        [[nodiscard]] bool isOutside(const Vector3i& pos) const;
        [[nodiscard]] int getWidth() const;

        [[nodiscard]] size_t getDepth() const {
            return depth;
        }

        [[nodiscard]] std::optional<Voxel> find(const Vector3i& pos) const;

        [[nodiscard]] size_t size() const {
            return nodes.size();
        }

        void dump() const;

    private:
        void insert(Node& parent, const Vector3i& origin, const Vector3i& pos, const Voxel& voxel, size_t level);
        [[nodiscard]] std::optional<Voxel> find(const Node& parent, const Vector3i& origin, const Vector3i& pos,
                                                size_t level) const;
        void dump(const Node& node, size_t level = 0) const;

        Pool<Node, maxNodesPerTree> nodes;
        size_t depth{0};
    };

    void insert(const Vector3i& pos, const Voxel& voxel) {
        voxels.insert(pos, voxel);
    }

private:
    Octree voxels;
};
} // namespace Engine
