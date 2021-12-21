#include "AssetModel.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "../Utils/GltfImporter.hpp"
#include "AssetManager.hpp"

#include <iostream>

using namespace Scissio;

static bool isCollision(const std::string& name) {
    if (name.find("_CCX") == name.size() - 4) {
        return true;
    }
    if (name.find("_CBO") == name.size() - 4) {
        return true;
    }
    return false;
}

template <typename T, typename It> std::optional<T> findOne(It begin, It end, const std::function<bool(const T&)>& fn) {
    for (auto it = begin; it != end; ++it) {
        if (fn(*it)) {
            return *it;
        }
    }
    return std::nullopt;
}

template <typename Container>
std::optional<typename Container::value_type>
findOne(const Container& container, const std::function<bool(const typename Container::value_type&)>& fn) {
    return findOne(container.begin(), container.end(), fn);
}

static std::tuple<std::optional<GltfNode>, std::optional<GltfNode>> validateGltfFile(const GltfImporter& gltf) {
    if (gltf.getMaterials().empty()) {
        EXCEPTION("gltf file has no materials");
    }

    const auto nodes = gltf.getNodes();
    if (nodes.empty()) {
        EXCEPTION("gltf file has no nodes");
    }

    const auto objects =
        findOne(nodes, [](const GltfNode& node) -> bool { return !isCollision(node.name) && node.mesh.has_value(); });
    const auto collisions =
        findOne(nodes, [](const GltfNode& node) -> bool { return isCollision(node.name) && node.mesh.has_value(); });

    if (!objects.has_value()) {
        EXCEPTION("gltf file has no valid node that can be used as a model");
    }

    const auto& object = objects.value();
    if (object.mesh.value().primitives.empty()) {
        EXCEPTION("gltf node has an object but contains no primitives");
    }

    for (const auto& primitive : object.mesh.value().primitives) {
        if (!primitive.indices.has_value()) {
            EXCEPTION("gltf object primitive has no indices");
        }

        if (!primitive.material.has_value()) {
            EXCEPTION("gltf object primitive has no material");
        }

        const auto positions = findOne(
            primitive.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Position; });
        const auto normals = findOne(
            primitive.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Normal; });
        const auto texCoords = findOne(
            primitive.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::TexCoord; });
        const auto tangents = findOne(
            primitive.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Tangent; });

        if (!positions.has_value()) {
            EXCEPTION("gltf object primitive has no position attribute");
        }
        if (!normals.has_value()) {
            EXCEPTION("gltf object primitive has no normals attribute");
        }
        if (!texCoords.has_value()) {
            EXCEPTION("gltf object primitive has no tex coords attribute");
        }
        if (!tangents.has_value()) {
            EXCEPTION("gltf object primitive has no tangents attribute");
        }

        const auto& indices = primitive.indices;

        if (indices->type != GltfType::Scalar) {
            EXCEPTION("gltf object primitive indicess must be a scalar type");
        }

        if (positions->accessor.type != GltfType::Vec3) {
            EXCEPTION("gltf object primitive positions must be a Vec3 type");
        }
        if (normals->accessor.type != GltfType::Vec3) {
            EXCEPTION("gltf object primitive normals must be a Vec3 type");
        }
        if (texCoords->accessor.type != GltfType::Vec2) {
            EXCEPTION("gltf object primitive tex coords must be a Vec3 type");
        }
        if (tangents->accessor.type != GltfType::Vec4) {
            EXCEPTION("gltf object primitive tangents must be a Vec3 type");
        }

        if (positions->accessor.componentType != GltfComponentType::R32f) {
            EXCEPTION("gltf object primitive positions must be component type of 32-bit float");
        }
        if (normals->accessor.componentType != GltfComponentType::R32f) {
            EXCEPTION("gltf object primitive normals must be component type of 32-bit float");
        }
        if (texCoords->accessor.componentType != GltfComponentType::R32f) {
            EXCEPTION("gltf object primitive tex coords must be component type of 32-bit float");
        }
        if (tangents->accessor.componentType != GltfComponentType::R32f) {
            EXCEPTION("gltf object primitive tangents must be component type of 32-bit float");
        }

        const auto count = positions->accessor.count;
        if (count != normals->accessor.count || count != texCoords->accessor.count ||
            count != tangents->accessor.count) {
            EXCEPTION("gltf object contains primitive which accessor has conflicting counts");
        }
    }

    return {
        std::optional<GltfNode>{object},
        !collisions.has_value() ? std::nullopt : std::optional<GltfNode>{collisions.value()},
    };
}

AssetModel::AssetModel(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path), primitives{}, bbMin{0}, bbMax{0}, bbRadius(0) {
}

void AssetModel::load(AssetManager& assetManager) {
    try {
        const GltfImporter gltf(path);
        const auto [object, collision] = validateGltfFile(gltf);

        const auto resolveTexture = [this, &assetManager](const std::string& filename,
                                                          const TextureType type) -> AssetTexturePtr {
            try {
                const auto baseName = Path(filename).stem().string();
                return assetManager.find<AssetTexture>(baseName);
            } catch (std::exception& e) {
                const auto file = path.parent_path() / Path(filename);
                const auto texture = assetManager.addTexture(getMod(), file, type);
                texture->load(assetManager);
                return texture;
            }
        };

        for (const auto& part : object.value().mesh.value().primitives) {
            const auto positions = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                                       return a.type == GltfAttributeType::Position;
                                   }).value();
            const auto normals = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                                     return a.type == GltfAttributeType::Normal;
                                 }).value();
            const auto texCoords = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                                       return a.type == GltfAttributeType::TexCoord;
                                   }).value();
            const auto tangents = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                                      return a.type == GltfAttributeType::Tangent;
                                  }).value();

            // Update the bounding box
            const auto min = Vector3(positions.accessor.min);
            const auto max = Vector3(positions.accessor.max);

            if (min.x < bbMin.x) {
                bbMin = Vector3{min.x, bbMin.y, bbMin.z};
            }
            if (min.y < bbMin.y) {
                bbMin = Vector3{bbMin.x, min.y, bbMin.z};
            }
            if (min.z < bbMin.z) {
                bbMin = Vector3{bbMin.x, bbMin.y, min.z};
            }

            if (max.x > bbMax.x) {
                bbMax = Vector3{max.x, bbMax.y, bbMax.z};
            }
            if (max.y > bbMax.y) {
                bbMax = Vector3{bbMax.x, max.y, bbMax.z};
            }
            if (max.z > bbMax.z) {
                bbMax = Vector3{bbMax.x, bbMax.y, max.z};
            }

            const auto& material = part.material.value();

            this->primitives.push_back({});
            Primitive& primitive = this->primitives.back();
            primitive.material.uniform.baseColorFactor = material.baseColorFactor;
            primitive.material.uniform.emissiveFactor = material.emissiveFactor;
            primitive.material.uniform.metallicRoughnessFactor = material.metallicRoughnessFactor;

            if (material.baseColorTexture.has_value()) {
                primitive.material.baseColorTexture =
                    resolveTexture(material.baseColorTexture.value().getUri(), TextureType::BaseColor);
            }

            if (material.normalTexture.has_value()) {
                primitive.material.normalTexture =
                    resolveTexture(material.normalTexture.value().getUri(), TextureType::Normals);
            }

            if (material.emissiveTexture.has_value()) {
                primitive.material.emissiveTexture =
                    resolveTexture(material.emissiveTexture.value().getUri(), TextureType::Emissive);
            }

            if (material.metallicRoughnessTexture.has_value()) {
                primitive.material.metallicRoughnessTexture =
                    resolveTexture(material.metallicRoughnessTexture.value().getUri(), TextureType::MetallicRoughness);
            }

            if (material.ambientOcclusionTexture.has_value()) {
                primitive.material.ambientOcclusionTexture =
                    resolveTexture(material.ambientOcclusionTexture.value().getUri(), TextureType::AmbientOcclusion);
            }

            auto indexType = IndexType::UnsignedByte;
            switch (part.indices.value().componentType) {
            case GltfComponentType::R8: {
                indexType = IndexType::UnsignedByte;
                break;
            }
            case GltfComponentType::R8u: {
                indexType = IndexType::UnsignedByte;
                break;
            }
            case GltfComponentType::R16: {
                indexType = IndexType::UnsignedShort;
                break;
            }
            case GltfComponentType::R16u: {
                indexType = IndexType::UnsignedShort;
                break;
            }
            case GltfComponentType::R32u: {
                indexType = IndexType::UnsignedInt;
                break;
            }
            default: {
                EXCEPTION("Invalid index type");
            }
            }

            auto primitiveType = PrimitiveType::Points;
            switch (part.type) {
            case GltfPrimitiveType::Lines: {
                primitiveType = PrimitiveType::Lines;
                break;
            }
            case GltfPrimitiveType::LineLoop: {
                primitiveType = PrimitiveType::LineLoop;
                break;
            }
            case GltfPrimitiveType::LineStrip: {
                primitiveType = PrimitiveType::LineStrip;
                break;
            }
            case GltfPrimitiveType::Triangles: {
                primitiveType = PrimitiveType::Triangles;
                break;
            }
            case GltfPrimitiveType::TriangleFan: {
                primitiveType = PrimitiveType::TriangleFan;
                break;
            }
            case GltfPrimitiveType::TriangleStrip: {
                primitiveType = PrimitiveType::TriangleStrip;
                break;
            }
            case GltfPrimitiveType::Points: {
                primitiveType = PrimitiveType::Points;
                break;
            }
            }

            const auto vboData =
                GltfUtils::combine(positions.accessor, normals.accessor, texCoords.accessor, tangents.accessor);

            const auto& iboData = part.indices->bufferView.getBuffer();

            primitive.mesh = Mesh{};

            primitive.vbo = VertexBuffer(VertexBufferType::Array);
            primitive.vbo.bufferData(vboData.data(), vboData.size() * sizeof(float), VertexBufferUsage::StaticDraw);
            primitive.mesh.addVertexBuffer(primitive.vbo, ShaderModel::Position{}, ShaderModel::Normal{},
                                           ShaderModel::TextureCoordinates{}, ShaderModel::Tangent{});

            primitive.ibo = VertexBuffer(VertexBufferType::Indices);
            primitive.ibo.bufferData(iboData.data(), iboData.size(), VertexBufferUsage::DynamicDraw);
            primitive.mesh.setIndexBuffer(primitive.ibo, indexType);

            primitive.ubo = VertexBuffer(VertexBufferType::Uniform);
            primitive.ubo.bufferData(&primitive.material.uniform, sizeof(Material::Uniform),
                                     VertexBufferUsage::StaticDraw);

            primitive.mesh.setPrimitive(primitiveType);
            primitive.mesh.setCount(static_cast<GLsizei>(part.indices->count));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            // primitives.push_back(std::move(primitive));
        }

        // Update the radius.
        // This is a smallest sphere that can encapsulate this entire model.
        bbRadius = std::max(std::abs(bbMin.x), bbRadius);
        bbRadius = std::max(std::abs(bbMin.y), bbRadius);
        bbRadius = std::max(std::abs(bbMin.z), bbRadius);
        bbRadius = std::max(std::abs(bbMax.x), bbRadius);
        bbRadius = std::max(std::abs(bbMax.y), bbRadius);
        bbRadius = std::max(std::abs(bbMax.z), bbRadius);

    } catch (...) {
        EXCEPTION_NESTED("Failed to load model: '{}'", getName());
    }
}

template <> void Xml::Node::convert<AssetModelPtr>(AssetModelPtr& value) const {
    value = AssetManager::singleton().find<AssetModel>(this->asString());
}

MSGPACK_UNPACK_FUNC(AssetModelPtr) {
    if (o.type == msgpack::type::STR) {
        const auto name = o.as<std::string>();
        v = AssetManager::singleton().find<AssetModel>(name);
    } else {
        throw msgpack::type_error();
    }

    return o;
}

MSGPACK_PACK_FUNC(AssetModelPtr) {
    if (v) {
        const auto& name = v->getName();
        o.pack_str(static_cast<uint32_t>(name.size()));
        o.pack_str_body(name.c_str(), static_cast<uint32_t>(name.size()));
    } else {
        o.pack_nil();
    }

    return o;
}
