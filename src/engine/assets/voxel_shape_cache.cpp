#include "voxel_shape_cache.hpp"
#include "../math/matrix.hpp"
#include "../utils/gltf_importer.hpp"

#define CMP "VoxelShapeCache"

using namespace Engine;

static VoxelShape::Face sideFromString(const std::string& str) {
    if (str == "X+") {
        return VoxelShape::Face::PositiveX;
    }
    if (str == "Y+") {
        return VoxelShape::Face::PositiveY;
    }
    if (str == "Z+") {
        return VoxelShape::Face::PositiveZ;
    }
    if (str == "X-") {
        return VoxelShape::Face::NegativeX;
    }
    if (str == "Y-") {
        return VoxelShape::Face::NegativeY;
    }
    if (str == "Z-") {
        return VoxelShape::Face::NegativeZ;
    }
    EXCEPTION("Unknown shape side name: '{}', must be one of [X+, X-, Y+, Y-, Z+, Z-]", str);
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

const Matrix4& VoxelShapeCache::getRotationMatrix(const uint8_t rotation) {
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

const Matrix4& VoxelShapeCache::getRotationMatrixInverted(const uint8_t rotation) {
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

static VoxelShape load(const Path& path) {
    try {
        VoxelShape shape{};

        const GltfImporter gltf(path);

        for (const auto& node : gltf.getNodes()) {
            VoxelShape::Face side = VoxelShape::Face::Default;
            if (node.name == "default") {
                EXCEPTION("TODO");
            } else {
                side = sideFromString(node.name);
            }

            if (!node.mesh.has_value()) {
                EXCEPTION("Node: '{}' has no mesh", node.name);
            }

            const auto& mesh = node.mesh.value();

            if (mesh.primitives.size() != 1) {
                EXCEPTION("Node: '{}' mesh: '{}' expected exactly one primitive", node.name, mesh.name);
            }

            const auto& primitive = mesh.primitives.front();

            if (primitive.type != GltfPrimitiveType::Triangles) {
                EXCEPTION("Node: '{}' mesh: '{}' expected primitive type of triangles", node.name, mesh.name);
            }

            if (!primitive.indices.has_value()) {
                EXCEPTION("Node: '{}' mesh: '{}' has no indices", node.name, mesh.name);
            }

            const auto& indices = primitive.indices.value();
            if (indices.type != GltfType::Scalar && indices.componentType != GltfComponentType::R16u) {
                EXCEPTION("Node: '{}' mesh: '{}' expected indices to be of uint16 type", node.name, mesh.name);
            }

            VoxelShape::Node data{};
            data.side = side;

            {
                data.indices.resize(indices.count);
                auto span = indices.bufferView.getSpan();
                std::memcpy(data.indices.data(), span.data(), span.size());
            }

            Span<Vector3> positions;
            Span<Vector3> normals;
            Span<Vector4> tangents;

            for (const auto& attr : primitive.attributes) {
                if (attr.type == GltfAttributeType::Position) {
                    if (attr.accessor.type != GltfType::Vec3 &&
                        attr.accessor.componentType != GltfComponentType::R32f) {
                        EXCEPTION("Node: '{}' mesh: '{}' expected positions to be of vec3 f32 type", node.name,
                                  mesh.name);
                    }

                    auto span = attr.accessor.bufferView.getSpan();
                    positions = Span<Vector3>{reinterpret_cast<const Vector3*>(span.data()), attr.accessor.count};

                } else if (attr.type == GltfAttributeType::Normal) {
                    if (attr.accessor.type != GltfType::Vec3 &&
                        attr.accessor.componentType != GltfComponentType::R32f) {
                        EXCEPTION("Node: '{}' mesh: '{}' expected normals to be of vec3 f32 type", node.name,
                                  mesh.name);
                    }

                    auto span = attr.accessor.bufferView.getSpan();
                    normals = Span<Vector3>{reinterpret_cast<const Vector3*>(span.data()), attr.accessor.count};

                } else if (attr.type == GltfAttributeType::Tangent) {
                    if (attr.accessor.type != GltfType::Vec4 &&
                        attr.accessor.componentType != GltfComponentType::R32f) {
                        EXCEPTION("Node: '{}' mesh: '{}' expected tangents to be of vec4 f32 type", node.name,
                                  mesh.name);
                    }

                    auto span = attr.accessor.bufferView.getSpan();
                    tangents = Span<Vector4>{reinterpret_cast<const Vector4*>(span.data()), attr.accessor.count};
                }
            }

            if (positions.empty()) {
                EXCEPTION("Node: '{}' mesh: '{}' has no positions", node.name, mesh.name);
            }
            if (normals.empty()) {
                EXCEPTION("Node: '{}' mesh: '{}' has no normals", node.name, mesh.name);
            }
            if (tangents.empty()) {
                EXCEPTION("Node: '{}' mesh: '{}' has no tangents", node.name, mesh.name);
            }

            if (positions.size() != normals.size() || normals.size() != tangents.size()) {
                EXCEPTION("Node: '{}' mesh: '{}' incorrect number of positions, normals, and tangents", node.name,
                          mesh.name);
            }

            data.vertices.resize(positions.size());
            const auto* position = positions.begin();
            const auto* normal = normals.begin();
            const auto* tangent = tangents.begin();

            for (size_t i = 0; i < positions.size(); i++) {
                data.vertices[i].position = *(position++);
                data.vertices[i].normal = *(normal++);
                // data.vertices[i].tangent = *(tangent++);
            }

            shape.sides.push_back(std::move(data));
        }

        return shape;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load shape: '{}'", path);
    }
}

VoxelShapeCache::VoxelShapeCache(const Config& config) : config{config} {
    init();
    precompute();
}

void VoxelShapeCache::init() {
    for (const auto shapeType : VoxelShape::allTypes) {
        const auto& name = shapeTypeToFileName(shapeType);
        const auto path = config.shapesPath / Path{name + ".gltf"};
        shapes.insert(std::make_pair(shapeType, load(path)));
    }
}

void VoxelShapeCache::precompute() {
    if (VoxelShape::typeNames.size() != shapes.size()) {
        EXCEPTION("shapeNames.size() != shapes.size()");
    }

    for (const auto shapeType : VoxelShape::allTypes) {
        // const auto& name = shapeTypeToFileName(shapeType);
        const auto loaded = shapes.at(shapeType);

        auto& shapeRotations = shapesPrebuilt.at(shapeType);

        // For all rotations
        for (size_t r = 0; r < 24; r++) {
            auto& shapeMasks = shapeRotations.at(r);
            const auto& rotationMatrix = getRotationMatrix(r);

            const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(rotationMatrix)));

            // For all mask variations
            for (uint8_t mask = 0; mask < 64; mask++) {
                auto& shape = shapeMasks.at(mask);

                // For all sides
                for (const auto& node : loaded.sides) {
                    if (node.side == VoxelShape::Face::Default) {
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
                    // v.tangent = Vector4{transformInverted * v.tangent, 0.0f};
                    // v.tangent = v.tangent;
                }
            }
        }
    }
}
