#include "AssetShape.hpp"
#include "../Utils/GltfImporter.hpp"
#include "AssetManager.hpp"

using namespace Engine;

static Shape::Side sideFromString(const std::string& str) {
    if (str == "X+") {
        return Shape::Side::PositiveX;
    }
    if (str == "Y+") {
        return Shape::Side::PositiveY;
    }
    if (str == "Z+") {
        return Shape::Side::PositiveZ;
    }
    if (str == "X-") {
        return Shape::Side::NegativeX;
    }
    if (str == "Y-") {
        return Shape::Side::NegativeY;
    }
    if (str == "Z-") {
        return Shape::Side::NegativeZ;
    }
    EXCEPTION("Unknown shape side name: '{}', must be one of [X+, X-, Y+, Y-, Z+, Z-]", str);
}

AssetShape::AssetShape(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path) {
}

void AssetShape::load(AssetManager& assetManager, bool noGraphics) {
    (void)noGraphics;

    try {
        const GltfImporter gltf(path);

        for (const auto& node : gltf.getNodes()) {
            Shape::Side side = Shape::Side::Default;
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

            Shape::Node data{};
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
                data.vertices[i].tangent = *(tangent++);
            }

            shape.sides.push_back(std::move(data));
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to load shape: '{}'", getName());
    }
}

std::shared_ptr<AssetShape> AssetShape::from(const std::string& name) {
    return AssetManager::singleton().find<AssetShape>(name);
}
