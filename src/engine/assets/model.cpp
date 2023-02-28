#include "model.hpp"
#include "../utils/gltf_importer.hpp"
#include "registry.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

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

Model::Model(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Model::load(Registry& registry, VulkanRenderer& vulkan) {
    (void)registry;

    const auto resolveTexture = [this, &registry](const std::string& filename) -> TexturePtr {
        try {
            const auto baseName = Path(filename).stem().string();
            return registry.getTextures().find(baseName);
        } catch (std::exception& e) {
            const auto file = path.parent_path() / Path(filename);
            return registry.addTexture(file);
        }
    };

    try {
        const GltfImporter gltf(path);

        if (gltf.getMaterials().empty()) {
            EXCEPTION("gltf file has no materials");
        }

        const auto nodes = gltf.getNodes();
        if (nodes.empty()) {
            EXCEPTION("gltf file has no nodes");
        }

        const auto objects = findOne(
            nodes, [](const GltfNode& node) -> bool { return !isCollision(node.name) && node.mesh.has_value(); });
        const auto collisions = findOne(
            nodes, [](const GltfNode& node) -> bool { return isCollision(node.name) && node.mesh.has_value(); });

        if (!objects.has_value()) {
            EXCEPTION("gltf file has no valid node that can be used as a model");
        }

        const auto& object = objects.value();
        if (object.mesh.value().primitives.empty()) {
            EXCEPTION("gltf node has an object but contains no primitives");
        }

        for (const auto& part : object.mesh.value().primitives) {
            if (!part.indices.has_value()) {
                EXCEPTION("gltf object primitive has no indices");
            }

            if (!part.material.has_value()) {
                EXCEPTION("gltf object primitive has no material");
            }

            const auto positions = findOne(
                part.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Position; });
            const auto normals = findOne(
                part.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Normal; });
            const auto texCoords = findOne(
                part.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::TexCoord; });
            const auto tangents = findOne(
                part.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Tangent; });

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

            const auto& indices = part.indices;

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

            // Update the bounding box
            const auto min = Vector3(positions->accessor.min);
            const auto max = Vector3(positions->accessor.max);

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

            const auto& partMaterial = part.material.value();

            this->primitives.push_back({});
            auto& primitive = this->primitives.back();
            this->materials.push_back({});
            auto& material = this->materials.back();
            primitive.material = &material;
            material.uniform.baseColorFactor = partMaterial.baseColorFactor;
            material.uniform.emissiveFactor = partMaterial.emissiveFactor;
            material.uniform.metallicRoughnessFactor = partMaterial.metallicRoughnessFactor;
            material.uniform.normalFactor = Vector4{1.0f};
            material.uniform.ambientOcclusionFactor = Vector4{1.0f};

            if (partMaterial.baseColorTexture) {
                material.baseColorTexture = resolveTexture(partMaterial.baseColorTexture.value().getUri());
            } else {
                material.baseColorTexture = registry.getDefaultTextures().baseColor;
            }

            if (partMaterial.normalTexture) {
                material.normalTexture = resolveTexture(partMaterial.normalTexture.value().getUri());
            } else {
                material.normalTexture = registry.getDefaultTextures().normal;
            }

            if (partMaterial.emissiveTexture) {
                material.emissiveTexture = resolveTexture(partMaterial.emissiveTexture.value().getUri());
            } else {
                material.emissiveTexture = registry.getDefaultTextures().emissive;
            }

            if (partMaterial.metallicRoughnessTexture) {
                material.metallicRoughnessTexture =
                    resolveTexture(partMaterial.metallicRoughnessTexture.value().getUri());
            } else {
                material.metallicRoughnessTexture = registry.getDefaultTextures().metallicRoughness;
            }

            if (partMaterial.ambientOcclusionTexture) {
                material.ambientOcclusionTexture =
                    resolveTexture(partMaterial.ambientOcclusionTexture.value().getUri());
            } else {
                material.ambientOcclusionTexture = registry.getDefaultTextures().ambient;
            }

            switch (part.indices.value().componentType) {
            case GltfComponentType::R8: {
                primitive.indexType = VkIndexType::VK_INDEX_TYPE_UINT8_EXT;
                break;
            }
            case GltfComponentType::R8u: {
                primitive.indexType = VkIndexType::VK_INDEX_TYPE_UINT8_EXT;
                break;
            }
            case GltfComponentType::R16: {
                primitive.indexType = VkIndexType::VK_INDEX_TYPE_UINT16;
                break;
            }
            case GltfComponentType::R16u: {
                primitive.indexType = VkIndexType::VK_INDEX_TYPE_UINT16;
                break;
            }
            case GltfComponentType::R32u: {
                primitive.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
                break;
            }
            default: {
                EXCEPTION("Invalid index type");
            }
            }

            if (part.type != GltfPrimitiveType::Triangles) {
                EXCEPTION("Invalid primitive type");
            }

            /*const auto vboData =
                GltfUtils::combine(positions->accessor, normals->accessor, texCoords->accessor, tangents->accessor);

            const auto& iboData = part.indices->bufferView.getBuffer();

            primitive.vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic,
                                                vboData.size() * sizeof(float));
            primitive.vbo.subData(vboData.data(), 0, vboData.size() * sizeof(float));

            primitive.ibo =
                vulkan.createBuffer(VulkanBuffer::Type::Index, VulkanBuffer::Usage::Dynamic, iboData.size());
            primitive.ibo.subData(iboData.data(), 0, iboData.size());

            material.ubo = vulkan.createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic,
                                               sizeof(Material::Uniform));
            material.ubo.subData(&material.uniform, 0, sizeof(Material::Uniform));

            primitive.count = static_cast<uint32_t>(part.indices->count);

            primitive.vboFormat = vulkan.createVertexInputFormat({
                {
                    0,
                    {
                        {0, 0, VulkanVertexInputFormat::Format::Vec3},
                        {1, 0, VulkanVertexInputFormat::Format::Vec3},
                        {2, 0, VulkanVertexInputFormat::Format::Vec2},
                        {3, 0, VulkanVertexInputFormat::Format::Vec4},
                    },
                },
            });*/
        }

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

ModelPtr Model::from(const std::string& name) {
    return Registry::getInstance().getModels().find(name);
}
