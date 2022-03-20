#include "Grid.hpp"

#define CMP "Grid"

using namespace Engine;

static int getWidthForLevel(const size_t level) {
    return static_cast<int>(std::pow(2, level)) / 2;
}

template <typename V> static inline glm::vec<3, V> idxToOffset(const int idx, const V s, const glm::vec<3, V>& origin) {
    using Vec = glm::vec<3, V>;

    switch (idx) {
    case 0: {
        return origin + Vec{s, s, s};
    }
    case 1: {
        return origin + Vec{-s, s, s};
    }
    case 2: {
        return origin + Vec{-s, s, -s};
    }
    case 3: {
        return origin + Vec{s, s, -s};
    }
    case 4: {
        return origin + Vec{s, -s, s};
    }
    case 5: {
        return origin + Vec{-s, -s, s};
    }
    case 6: {
        return origin + Vec{-s, -s, -s};
    }
    case 7: {
        return origin + Vec{s, -s, -s};
    }
    default: {
        throw std::out_of_range("Invalid index");
    }
    }
}

static inline size_t idxToMove(const int idx) {
    switch (idx) {
    case 0: {
        return 6;
    }
    case 1: {
        return 7;
    }
    case 2: {
        return 4;
    }
    case 3: {
        return 5;
    }
    case 4: {
        return 2;
    }
    case 5: {
        return 3;
    }
    case 6: {
        return 0;
    }
    case 7: {
        return 1;
    }
    default: {
        EXCEPTION("Invalid index to calculate move index");
    }
    }
}

static uint8_t posToIndex(const Vector3i& pos, const Vector3i& origin) {
    if (pos.y >= origin.y) {
        // Top layer
        if (pos.x >= origin.x) {
            if (pos.z >= origin.z) {
                return 0;
            } else {
                return 3;
            }
        } else {
            if (pos.z >= origin.z) {
                return 1;
            } else {
                return 2;
            }
        }
    } else {
        // Bottom layer
        if (pos.x >= origin.x) {
            if (pos.z >= origin.z) {
                return 4;
            } else {
                return 7;
            }
        } else {
            if (pos.z >= origin.z) {
                return 5;
            } else {
                return 6;
            }
        }
    }
}

Grid::Octree::Octree() {
    depth = 1;
    auto& root = nodes.insert();
    root.branch.index = 0;
    root.branch.next = badIndex;
    root.branch.child = badIndex;
    // Log::d(CMP, "Octree() root: {}", root.branch.string());
    // dump();
}

void Grid::Octree::insert(const Vector3i& pos, const Voxel& voxel) {
    while (isOutside(pos)) {
        expand(0, 1);
        // Log::d(CMP, "After expand");
        // dump();
    }

    insert(nodes.at(0), Vector3i{0}, pos, voxel, 1);
}

void Grid::Octree::insert(Node& parent, const Vector3i& origin, const Vector3i& pos, const Voxel& voxel,
                          const size_t level) {
    const auto parentIdx = nodes.indexOf(parent);
    Index childIdx = parent.branch.child;
    const auto insertIdx = posToIndex(pos, origin);
    Index lastChildIdx = 0;

    while (childIdx) {
        if (!childIdx || childIdx == badIndex) {
            break;
        }

        lastChildIdx = childIdx;
        auto& child = nodes.at(childIdx);

        /*if (depth - level == 0) {
            child.setType()
        }*/

        if (depth - level == 0) {
            if (child.voxel.index == insertIdx) {
                child.voxel.color = voxel.color;
                child.voxel.rotation = voxel.rotation;
                child.voxel.type = voxel.type;
                return;
            }
        } else {
            if (child.branch.index == insertIdx) {
                const auto newOrigin = idxToOffset(insertIdx, getWidthForLevel(depth - level + 1) / 2, origin);
                // Log::d(CMP, "insert branch newOrigin: [{}, {}, {}]", newOrigin.x, newOrigin.y, newOrigin.z);
                insert(child, newOrigin, pos, voxel, level + 1);
                return;
            }
        }

        childIdx = child.branch.next;
    }

    // Insert new node
    auto& child = nodes.insert();
    if (depth - level == 0) {
        // Log::d(CMP, "Inserting new child voxel with index: {}", insertIdx);
        child.voxel.index = insertIdx;
        child.voxel.next = badIndex;
        child.voxel.color = voxel.color;
        child.voxel.rotation = voxel.rotation;
        child.voxel.type = voxel.type;
    } else {
        // Log::d(CMP, "Inserting new child branch with index: {}", insertIdx);
        child.branch.child = badIndex;
        child.branch.index = insertIdx;
        child.branch.next = badIndex;
    }
    const auto newChildIdx = nodes.indexOf(child);

    if (!lastChildIdx) {
        // Log::d(CMP, "parentIdx: {} newChildIdx: {}", parentIdx, newChildIdx);
        nodes.at(parentIdx).branch.child = newChildIdx;
    } else {
        if (depth - level == 0) {
            // Log::d(CMP, "lastChildIdx (voxel): {} newChildIdx: {}", parentIdx, newChildIdx);
            nodes.at(lastChildIdx).voxel.next = newChildIdx;
        } else {
            // Log::d(CMP, "lastChildIdx (branch): {} newChildIdx: {}", parentIdx, newChildIdx);
            nodes.at(lastChildIdx).branch.next = newChildIdx;
        }
    }

    if (depth - level != 0) {
        const auto newOrigin = idxToOffset(insertIdx, getWidthForLevel(depth - level + 1) / 2, origin);
        // Log::d(CMP, "insert new newOrigin: [{}, {}, {}]", newOrigin.x, newOrigin.y, newOrigin.z);
        insert(child, newOrigin, pos, voxel, level + 1);
    }
}

std::optional<Grid::Voxel> Grid::Octree::find(const Vector3i& pos) const {
    if (isOutside(pos)) {
        return std::nullopt;
    }
    return find(nodes.at(0), Vector3i{0}, pos, 1);
}

std::optional<Grid::Voxel> Grid::Octree::find(const Node& parent, const Vector3i& origin, const Vector3i& pos,
                                              const size_t level) const {
    Index childIdx = parent.branch.child;
    const auto findIdx = posToIndex(pos, origin);

    // Log::d(CMP, "Got first child idx: {}", childIdx);

    while (childIdx) {
        if (!childIdx || childIdx == badIndex) {
            break;
        }

        auto& child = nodes.at(childIdx);

        if (depth - level == 0) {
            // Log::d(CMP, "Checking if child index {} matches expected index {}", child.voxel.index, findIdx);
            if (child.voxel.index == findIdx) {
                return child.voxel;
            }
        } else {
            // Log::d(CMP, "Checking if child index {} matches expected index {}", child.branch.index, findIdx);
            if (child.branch.index == findIdx) {
                const auto newOrigin = idxToOffset(findIdx, getWidthForLevel(depth - level + 1) / 2, origin);
                // Log::d(CMP, "find newOrigin: [{}, {}, {}]", newOrigin.x, newOrigin.y, newOrigin.z);
                return find(child, newOrigin, pos, level + 1);
            }
        }

        childIdx = child.branch.next;
        // Log::d(CMP, "Got next child idx: {}", childIdx);
    }

    return std::nullopt;
}

void Grid::Octree::expand(const Index idx, const size_t level) {
    // std::vector<Node*> children;
    // children.reserve(8);

    auto& parent = nodes.at(idx);
    // Log::d(CMP, "Expanding parent: {}", parent.branch.string());
    Index childIdx = parent.branch.child;

    parent.branch.child = 0;
    Index lastNewChildIdx = 0;

    const auto addChildToParent = [&](Index grandChildIdx) {
        auto& newChild = nodes.insert();
        newChild.branch.child = grandChildIdx;

        if (nodes.at(idx).branch.child == 0) {
            lastNewChildIdx = nodes.indexOf(newChild);
            nodes.at(idx).branch.child = lastNewChildIdx;
        } else {
            auto& sibling = nodes.at(lastNewChildIdx);
            lastNewChildIdx = nodes.indexOf(newChild);
            sibling.branch.next = lastNewChildIdx;
        }
    };

    while (childIdx && childIdx != badIndex) {
        auto& child = nodes.at(childIdx);
        // child.branch.setNext(0);
        // children.push_back(&child);

        Index nextChild;
        if (depth - level == 0) {
            nextChild = child.voxel.next;
            child.voxel.next = badIndex;
            child.voxel.index = idxToMove(child.voxel.index);
        } else {
            nextChild = child.branch.next;
            child.branch.next = badIndex;
            child.branch.index = idxToMove(child.branch.index);
        }

        addChildToParent(childIdx);

        childIdx = nextChild;
    }

    depth++;
}

void Grid::Octree::dump() const {
    dump(nodes.at(0));
}

void Grid::Octree::dump(const Grid::Node& node, size_t level) const {
    std::string indent(level * 2, ' ');
    if (depth - level == 0) {
        // Log::d(CMP, "{}Voxel: [{}] {}", indent, nodes.indexOf(node), node.voxel.string());

        Index nextIdx = node.voxel.next;
        while (nextIdx && nextIdx != badIndex) {
            const auto& next = nodes.at(nextIdx);
            dump(next, level);
            nextIdx = next.voxel.next;
        }
    } else {
        // Log::d(CMP, "{}Branch: [{}] {}", indent, nodes.indexOf(node), node.branch.string());

        Index childIdx = node.branch.child;
        if (childIdx != badIndex) {
            dump(nodes.at(childIdx), level + 1);
        }

        Index nextIdx = node.branch.next;
        while (nextIdx && nextIdx != badIndex) {
            const auto& next = nodes.at(nextIdx);
            dump(next, level);
            nextIdx = next.branch.next;
        }
    }
}

bool Grid::Octree::isOutside(const Vector3i& pos) const {
    const auto w = getWidth();
    if (pos.x >= w || pos.x < -w)
        return true;
    if (pos.y >= w || pos.y < -w)
        return true;
    if (pos.z >= w || pos.z < -w)
        return true;
    return false;
}

int Grid::Octree::getWidth() const {
    return getWidthForLevel(depth);
}
