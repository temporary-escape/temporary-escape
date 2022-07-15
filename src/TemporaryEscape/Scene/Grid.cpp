#include "Grid.hpp"
#include "../Assets/AssetManager.hpp"

#define CMP "Grid"

using namespace Engine;

static inline size_t coordToIdx(const Vector3i& pos, size_t width) {
    // return x + Grid::cacheBuildWidth * (y + Grid::cacheBuildWidth * z);

    return pos.x + (pos.y * width) + (pos.z * width * width);
}

template <typename V> static inline glm::vec<3, V> idxToOffset(const int idx, const V s, const glm::vec<3, V>& origin) {
    // Log::d(CMP, "idxToOffset({}, {}, {})", idx, s, origin);
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

static Vector3 vecToNormal(const Vector3& vec) {
    if (vec.x >= 0.49f) {
        return Vector3{1.0f, 0.0f, 0.0f};
    }
    if (vec.x <= -0.49f) {
        return Vector3{-1.0f, 0.0f, 0.0f};
    }
    if (vec.y >= 0.49f) {
        return Vector3{0.0f, 1.0f, 0.0f};
    }
    if (vec.y <= -0.49f) {
        return Vector3{0.0f, -1.0f, 0.0f};
    }
    if (vec.z >= 0.49f) {
        return Vector3{0.0f, 0.0f, 1.0f};
    }
    return Vector3{0.0f, 0.0f, -1.0f};
}

static uint8_t enumToMask[7] = {
    0x00, // Default
    0x01, // PositiveX
    0x02, // NegativeX
    0x04, // PositiveY
    0x08, // NegativeY
    0x10, // PositiveZ
    0x20, // NegativeZ
};

const Matrix4& Grid::getRotationMatrix(const uint8_t rotation) {
    static std::vector<Matrix4> matrices;

    static std::array<Matrix4, 4> orientationsA = {
        glm::rotate(glm::radians(0.0f), Vector3{0.0f, 1.0f, 0.0f}),
        glm::rotate(glm::radians(90.0f), Vector3{0.0f, 1.0f, 0.0f}),
        glm::rotate(glm::radians(180.0f), Vector3{0.0f, 1.0f, 0.0f}),
        glm::rotate(glm::radians(270.0f), Vector3{0.0f, 1.0f, 0.0f}),
    };

    static std::array<Matrix4, 6> orientationsB = {
        glm::rotate(glm::radians(0.0f), Vector3{0.0f, 0.0f, 1.0f}),
        glm::rotate(glm::radians(90.0f), Vector3{0.0f, 0.0f, 1.0f}),
        glm::rotate(glm::radians(180.0f), Vector3{0.0f, 0.0f, 1.0f}),
        glm::rotate(glm::radians(270.0f), Vector3{0.0f, 0.0f, 1.0f}),
        glm::rotate(glm::radians(90.0f), Vector3{1.0f, 0.0f, 0.0f}),
        glm::rotate(glm::radians(-90.0f), Vector3{1.0f, 0.0f, 0.0f}),
    };

    if (rotation >= 24) {
        EXCEPTION("Bad rotation number: {}, must be between [0, 23]", rotation);
    }

    if (matrices.empty()) {
        for (auto i = 0; i < 24; i++) {
            const auto a = i / 6;
            const auto b = i % 6;

            matrices.push_back(orientationsB.at(b) * orientationsA.at(a));
        }
    }

    return matrices.at(rotation);
}

const Matrix4& Grid::getRotationMatrixInverted(const uint8_t rotation) {
    static std::vector<Matrix4> matrices;

    if (rotation >= 24) {
        EXCEPTION("Bad rotation number: {}, must be between [0, 23]", rotation);
    }

    if (matrices.empty()) {
        for (auto r = 0; r < 24; r++) {
            matrices.push_back(glm::inverse(getRotationMatrix(r)));
        }
    }

    return matrices.at(rotation);
}

Grid::Builder::Builder(AssetManager& assetManager) : assetManager(assetManager) {
}

void Grid::Builder::precompute() {
    if (Shape::typeNames.size() != shapes.size()) {
        EXCEPTION("shapeNames.size() != shapes.size()");
    }

    for (const auto shapeType : Shape::allTypes) {
        const auto& name = shapeTypeToFileName(shapeType);
        const auto asset = assetManager.find<AssetShape>(name);

        auto& shapeRotations = shapes.at(shapeType);

        // For all rotations
        for (size_t r = 0; r < 24; r++) {
            auto& shapeMasks = shapeRotations.at(r);
            const auto& rotationMatrix = getRotationMatrix(r);

            const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(rotationMatrix)));

            // For all mask variations
            for (uint8_t mask = 0; mask < 64; mask++) {
                auto& shape = shapeMasks.at(mask);

                // For all sides
                for (const auto& node : asset->getShape().sides) {
                    if (node.side == Shape::Side::Default) {
                        EXCEPTION("TODO");
                    }

                    if ((mask & enumToMask[node.side]) == 0) {
                        continue;
                    }

                    const auto vertexOffset = shape.vertices.size();
                    shape.vertices.reserve(vertexOffset + node.vertices.size());
                    for (const auto& v : node.vertices) {
                        shape.vertices.push_back(v);
                    }

                    const auto indexOffset = shape.indices.size();
                    shape.indices.reserve(indexOffset + node.indices.size());
                    for (const auto& i : node.indices) {
                        shape.indices.push_back(i + vertexOffset);
                    }
                }

                for (auto& v : shape.vertices) {
                    v.position = Vector3{rotationMatrix * Vector4{v.position, 1.0f}};
                    v.normal = Vector3{transformInverted * Vector4{v.normal, 0.0f}};
                    v.tangent = Vector4{transformInverted * v.tangent, 0.0f};
                    // v.tangent = v.tangent;
                }
            }
        }
    }
}

void Grid::Builder::build(const Voxel* cache, const std::vector<Type>& types, RawPrimitiveData& map,
                          const Vector3& offset) {
    static const Vector3i neighbourOffset[6] = {
        Vector3i{1, 0, 0},  Vector3i{-1, 0, 0}, Vector3i{0, 1, 0},
        Vector3i{0, -1, 0}, Vector3i{0, 0, -1}, Vector3i{0, 0, 1},
    };

    static const Shape::Side neighbourSides[6] = {
        Shape::Side::PositiveX, Shape::Side::NegativeX, Shape::Side::PositiveY,
        Shape::Side::NegativeY, Shape::Side::PositiveZ, Shape::Side::NegativeZ,
    };

    static const uint8_t neighbourMasks[6] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
    };

    static const uint8_t shapeSolidMask[4] = {
        0b00111111,
        0b00000000,
        0b00000000,
        0b00000000,
    };

    static const auto generateRotatedMasks = []() {
        std::array<std::array<uint8_t, 24>, 64> results{};

        // For each neighbour combination
        for (auto m = 0; m < 64; m++) {

            // For each rotation
            for (auto r = 0; r < 24; r++) {
                uint8_t result = 0;

                // For each neighbour
                for (auto n = 0; n < 6; n++) {
                    if ((m & neighbourMasks[n]) == 0) {
                        continue;
                    }

                    const auto rotated = Vector3{getRotationMatrixInverted(r) * Vector4{neighbourOffset[n], 0.0f}};

                    // Find the correct destination bit
                    auto index = -1;
                    for (auto t = 0; t < 6; t++) {
                        const auto d = glm::dot(rotated, Vector3{neighbourOffset[t]});

                        if (d > 0.95f) {
                            index = t;
                            break;
                        }
                    }

                    if (index == -1) {
                        EXCEPTION("This should not happen");
                    }

                    result |= (1 << index);
                }

                results[m][r] = result;
            }
        }

        return results;
    };

    static std::array<std::array<uint8_t, 24>, 64> rotatedMasks = generateRotatedMasks();

    const auto typeSideToMaterial = [&](const uint16_t type, const uint8_t side) {};

    const auto appendShapeVertices = [&](RawPrimitiveData::mapped_type& data, const ShapePrebuilt& shape,
                                         const Vector3& pos) {
        const auto vertexOffset = data.vertices.size();
        data.vertices.reserve(vertexOffset + shape.vertices.size());
        for (const auto& v : shape.vertices) {
            data.vertices.push_back(v);
            data.vertices.back().position += pos + offset;
        }

        const auto indexOffset = data.indices.size();
        data.indices.reserve(indexOffset + shape.indices.size());
        for (const auto& i : shape.indices) {
            data.indices.push_back(i + vertexOffset);
        }
    };

    for (auto z = 0; z < meshBuildWidth; z++) {
        for (auto y = 0; y < meshBuildWidth; y++) {
            for (auto x = 0; x < meshBuildWidth; x++) {
                const auto pos = Vector3i{x, y, z};
                const auto& item = cache[coordToIdx(pos + Vector3i{1}, cacheBuildWidth)];
                if (!item) {
                    continue;
                }

                if (uint16_t(item.type) >= types.size()) {
                    continue;
                }

                // Log::d(CMP, "building voxel pos: [{}, {}, {}]", x, y, z);

                const auto posf = Vector3{pos};
                const auto& block = types.at(item.type).block;

                // auto& data = map[item.type];

                uint8_t mask = 0;

                for (size_t n = 0; n < 6; n++) {
                    const auto& neighbour = cache[coordToIdx(pos + neighbourOffset[n] + Vector3i{1}, cacheBuildWidth)];
                    if (!neighbour || !(shapeSolidMask[neighbour.shape] & neighbourMasks[n])) {
                        mask = mask | neighbourMasks[n];
                    }
                }

                // Rotate mask
                mask = rotatedMasks[mask][item.rotation];

                if (block->isSingular()) {
                    const auto& shape = shapes.at(item.shape).at(item.rotation).at(mask);
                    auto& data = map[&block->getMaterial()];
                    appendShapeVertices(data, shape, posf);
                } else {
                    for (size_t n = 0; n < 6; n++) {
                        const auto shapeMask = mask & neighbourMasks[n];
                        if (!shapeMask) {
                            continue;
                        }
                        const auto& shape = shapes.at(item.shape).at(item.rotation).at(shapeMask);

                        const auto& material = block->getMaterialForSide(neighbourSides[n]);

                        auto& data = map[&material];
                        appendShapeVertices(data, shape, posf);
                    }
                }
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

    Log::d(CMP, "Inserting pos: {}", pos);
    insert(nodes.at(0), Vector3i{0}, pos, voxel, 1);
}

void Grid::Octree::insert(Node& parent, const Vector3i& origin, const Vector3i& pos, const Voxel& voxel,
                          const size_t level) {
    const auto parentIdx = nodes.indexOf(parent);
    Index childIdx = parent.branch.child;
    const auto insertIdx = posToIndex(pos, origin);
    Index lastChildIdx = 0;

    Log::d(CMP, "Insert at origin: {} pos: {} level: {}", origin, pos, level);

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
                child.voxel.shape = voxel.shape;
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
        child.voxel.shape = voxel.shape;
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

std::optional<Grid::RayCastResult> Grid::Octree::rayCast(const Vector3& from, const Vector3& to) const {

    return rayCast(nodes.at(0), Vector3i{0}, 1, from, to);
}

std::optional<Grid::RayCastResult> Grid::Octree::rayCast(const Grid::Node& parent, const Vector3i& origin, size_t level,
                                                         const Vector3& from, const Vector3& to) const {
    std::optional<Grid::RayCastResult> result;

    Index childIdx = parent.branch.child;
    while (childIdx) {
        if (!childIdx || childIdx == badIndex) {
            break;
        }

        auto& child = nodes.at(childIdx);

        const auto index = depth - level == 0 ? child.voxel.index : child.branch.index;

        const auto half = static_cast<float>(getWidthForLevel(depth - level + 1)) / 2.0f;
        const auto childPos = idxToOffset<float>(index, half, origin);

        const auto min = Vector3{childPos} - Vector3{half} - Vector3{0.5f};
        const auto max = Vector3{childPos} + Vector3{half} - Vector3{0.5f};

        const auto pos = intersectBox(min, max, from, to);
        if (pos) {
            if (depth - level == 0) {
                // Bottom
                if (!result || glm::distance(result.value().hitPos, from) > glm::distance(pos.value(), from)) {
                    const auto worldChildPos = (max + min) / 2.0f;
                    const auto diff = pos.value() - worldChildPos;
                    result = RayCastResult{child, vecToNormal(diff), pos.value(), childPos, worldChildPos};
                }
            } else {
                const auto newOrigin = idxToOffset(index, getWidthForLevel(depth - level + 1) / 2, origin);
                auto test = rayCast(child, newOrigin, level + 1, from, to);
                if (test) {
                    if (!result ||
                        glm::distance(result.value().hitPos, from) > glm::distance(test.value().hitPos, from)) {
                        result = test;
                    }
                }
            }
        }

        childIdx = child.branch.next;
    }

    return result;
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

    parent.branch.child = badIndex;
    Index lastNewChildIdx = badIndex;

    const auto addChildToParent = [&](Index grandChildIdx) {
        // Log::d(CMP, "addChildToParent() grandChildIdx: {}", grandChildIdx);
        auto& newChild = nodes.insert();
        newChild.branch.child = grandChildIdx;
        newChild.branch.next = badIndex;

        if (nodes.at(idx).branch.child == badIndex) {
            lastNewChildIdx = nodes.indexOf(newChild);
            nodes.at(idx).branch.child = lastNewChildIdx;
        } else {
            auto& sibling = nodes.at(lastNewChildIdx);
            lastNewChildIdx = nodes.indexOf(newChild);
            sibling.branch.next = lastNewChildIdx;
        }
    };

    while (childIdx != badIndex) {
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
        Log::d(CMP, "{}Voxel: [{}] {}", indent, nodes.indexOf(node), node.voxel.string());

        Index nextIdx = node.voxel.next;
        while (nextIdx && nextIdx != badIndex) {
            const auto& next = nodes.at(nextIdx);
            dump(next, level);
            nextIdx = next.voxel.next;
        }
    } else {
        Log::d(CMP, "{}Branch: [{}] {}", indent, nodes.indexOf(node), node.branch.string());

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

Grid::Iterator::Iterator(Octree& octree, const size_t idx, size_t level, const Vector3i& origin)
    : octree(octree), idx(idx), level(level), origin(origin) {

    if (level == 0) {
        pos = origin;
    } else if (idx != badIndex && isVoxel()) {
        pos = idxToOffset(value().voxel.index, getBranchWidth() / 2.0f, Vector3{origin});
    } else if (idx != badIndex) {
        pos = idxToOffset(value().branch.index, getBranchWidth() / 2.0f, Vector3{origin});
    }
}

void Grid::Iterator::next() {
    if (octree.getDepth() - level == 0) {
        idx = octree.pool().at(idx).voxel.next;
    } else {
        idx = octree.pool().at(idx).branch.next;
    }

    if (idx != badIndex) {
        if (isVoxel()) {
            pos = idxToOffset(value().voxel.index, getBranchWidth() / 2.0f, Vector3{origin});
        } else {
            pos = idxToOffset(value().branch.index, getBranchWidth() / 2.0f, Vector3{origin});
        }
    }
}

Grid::Iterator Grid::Iterator::children() {
    if (bool(*this) && octree.getDepth() - level > 0) {
        return Iterator(octree, value().branch.child, level + 1, pos);
    }
    return Iterator(octree, badIndex, level + 1, Vector3i{0});
}

void Grid::insert(const Vector3i& pos, const AssetBlockPtr& block, const uint8_t rotation, const uint8_t color,
                  const uint8_t shape) {
    const auto type = insertBlock(block);
    Voxel voxel{};
    voxel.type = type;
    voxel.color = color;
    voxel.rotation = rotation;
    voxel.shape = shape;
    voxels.insert(pos, voxel);
}

uint16_t Grid::insertBlock(const AssetBlockPtr& block) {
    for (size_t i = 0; i < types.size(); i++) {
        auto& type = types.at(i);
        if (type.block == block) {
            ++type.count;
            return i;
        }
    }

    types.emplace_back();
    auto& type = types.back();
    type.block = block;
    type.count = 1;
    return types.size() - 1;
}

void Grid::generateMesh(Grid::Builder& gridBuilder, Grid::Builder::RawPrimitiveData& map) {
    auto it = voxels.iterate();
    generateMesh(it, gridBuilder, map);
}

void Grid::generateMesh(Iterator& iterator, Grid::Builder& gridBuilder, Grid::Builder::RawPrimitiveData& map) {
    if (!iterator) {
        return;
    }

    while (iterator) {
        auto pos = iterator.getPos();

        if (!iterator.isVoxel()) {
            if (iterator.getBranchWidth() <= meshBuildWidth) {
                generateMeshBlock(iterator, gridBuilder, map);
            } else {
                auto children = iterator.children();
                generateMesh(children, gridBuilder, map);
            }
        }

        iterator.next();
    }
}

void Grid::generateMeshBlock(Iterator& iterator, Grid::Builder& gridBuilder, Grid::Builder::RawPrimitiveData& map) {
    if (!iterator) {
        return;
    }

    std::unique_ptr<Voxel[]> cache(new Voxel[cacheBuildWidth * cacheBuildWidth * cacheBuildWidth]);
    std::memset(cache.get(), 0x00, sizeof(Voxel) * cacheBuildWidth * cacheBuildWidth * cacheBuildWidth);

    const auto& origin = iterator.getPos();
    Log::d(CMP, "generateMeshBlock origin: {}", origin);
    const auto min = origin - Vector3i{iterator.getBranchWidth() / 2};

    auto children = iterator.children();
    generateMeshCache(children, cache.get(), min);

    Log::d(CMP, "generateMeshBlock build min: {}", min);
    gridBuilder.build(cache.get(), types, map, min);
}

void Grid::generateMeshCache(Iterator& iterator, Voxel* cache, const Vector3i& offset) const {
    if (!iterator) {
        return;
    }

    while (iterator) {
        Log::d(CMP, "buildMesh iterator pos: {} offset: {}", iterator.getPos(), offset);
        const auto pos = iterator.getPos() - offset;

        if (!iterator.isVoxel()) {
            auto children = iterator.children();
            generateMeshCache(children, cache, offset);
        } else {
            Log::d(CMP, "buildMesh: [{}] {} pos: [{}, {}, {}]", voxels.pool().indexOf(iterator.value()),
                   iterator.value().voxel.string(), pos.x, pos.y, pos.z);

            auto& dst = cache[coordToIdx(pos + Vector3i{1}, cacheBuildWidth)];
            dst = iterator.value().voxel;
        }

        iterator.next();
    }
}

std::optional<Grid::RayCastResult> Grid::rayCast(const Vector3& from, const Vector3& to) const {
    return voxels.rayCast(from, to);
}
