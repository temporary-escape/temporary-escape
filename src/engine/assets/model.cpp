#include "model.hpp"
#include "../file/gltf_file_reader.hpp"
#include "../server/lua.hpp"
#include "../utils/string_utils.hpp"
#include "assets_manager.hpp"
#include <btBulletDynamicsCommon.h>
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static const std::vector<Vector3> boxVertices = {
    {-1.0f, 1.0f, -1.0f}, // 0
    {-1.0f, 1.0f, 1.0f},  // 1
    {1.0f, 1.0f, -1.0f},  // 2
    {1.0f, 1.0f, 1.0f},   // 3

    {-1.0f, -1.0f, -1.0f}, // 4
    {-1.0f, -1.0f, 1.0f},  // 5
    {1.0f, -1.0f, -1.0f},  // 6
    {1.0f, -1.0f, 1.0f},   // 7
};

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

template <typename Container>
std::vector<typename Container::value_type>
findMany(const Container& container, const std::function<bool(const typename Container::value_type&)>& fn) {
    std::vector<typename Container::value_type> res;
    std::copy_if(container.begin(), container.end(), std::back_inserter(res), fn);
    return res;
}

static Path fixFileNameExt(const Path& path) {
    const auto filename = fmt::format("{}.ktx2", path.stem().string());
    if (path.has_parent_path()) {
        return path.parent_path() / filename;
    }
    return filename;
}

Model::Model(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

Model::Model(Model&& other) noexcept = default;

Model& Model::operator=(Model&& other) noexcept = default;

Model::~Model() = default;

void Model::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)audio;

    const auto resolveTexture = [this, &assetsManager](const Path& filename) -> TexturePtr {
        const auto baseName = filename.stem().string();
        auto found = assetsManager.getTextures().findOrNull(baseName);
        if (found) {
            return found;
        }
        const auto file = path.parent_path() / fixFileNameExt(filename);
        return assetsManager.addTexture(file);
    };

    try {
        const GltfFileReader gltf{path};

        if (gltf.getMaterials().empty()) {
            EXCEPTION("gltf file has no materials");
        }

        const auto& modelNodes = gltf.getNodes();
        if (modelNodes.empty()) {
            EXCEPTION("gltf file has no nodes");
        }

        const auto objects = findMany(
            modelNodes, [](const GltfNode& node) -> bool { return !isCollision(node.name) && node.mesh.has_value(); });

        const auto collisions = findOne(
            modelNodes, [](const GltfNode& node) -> bool { return isCollision(node.name) && node.mesh.has_value(); });

        if (objects.empty()) {
            EXCEPTION("gltf file has no valid node that can be used as a model");
        }

        for (const auto& object : objects) {
            if (object.mesh.value().primitives.empty()) {
                EXCEPTION("gltf node has an object but contains no primitives");
            }

            const auto& skin = object.skin;

            auto& node = nodes.emplace_back();
            node.name = object.name;

            if (skin) {
                if (skin->inverseBindMatrices.type != GltfType::Mat4) {
                    EXCEPTION("gltf object skin matrices must be a mat4 type");
                }
                if (skin->inverseBindMatrices.count > maxJoints) {
                    EXCEPTION("gltf object skin can not have more than {} joints", maxJoints);
                }

                node.skin.count = skin->inverseBindMatrices.count;
                if (node.skin.count != skin->joints.size()) {
                    EXCEPTION("gltf object skin joints count does not match nodes count");
                }

                auto buffer = skin->inverseBindMatrices.bufferView.getSpan();
                if (node.skin.count * sizeof(Matrix4) != buffer.size()) {
                    EXCEPTION("gltf object skin matrices buffer does not match the expected size");
                }

                std::memcpy(node.skin.inverseBindMat.data(), buffer.data(), node.skin.count * sizeof(Matrix4));

                for (size_t i = 0; i < skin->joints.size(); i++) {
                    const auto& joint = skin->joints[i];
                    auto& mat = node.skin.jointsLocalMat[i];

                    mat = Matrix4{1.0f};
                    mat = glm::translate(mat, joint.translation);
                    mat = mat * glm::mat4_cast(joint.rotation);
                }
            }

            for (const auto& part : object.mesh.value().primitives) {
                if (!part.indices.has_value()) {
                    EXCEPTION("gltf object primitive has no indices");
                }

                if (!part.material.has_value()) {
                    EXCEPTION("gltf object primitive has no material");
                }

                const auto positions = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                    return a.type == GltfAttributeType::Position;
                });
                const auto normals = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                    return a.type == GltfAttributeType::Normal;
                });
                const auto texCoords = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                    return a.type == GltfAttributeType::TexCoord;
                });
                const auto tangents = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                    return a.type == GltfAttributeType::Tangent;
                });
                const auto joints = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                    return a.type == GltfAttributeType::Joints;
                });
                const auto weights = findOne(part.attributes, [](const GltfAttribute& a) -> bool {
                    return a.type == GltfAttributeType::Weights;
                });

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
                if (skin && !joints.has_value()) {
                    EXCEPTION("gltf object primitive has no joints attribute");
                }
                if (skin && !weights.has_value()) {
                    EXCEPTION("gltf object primitive has no weights attribute");
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
                    EXCEPTION("gltf object primitive tex coords must be a Vec2 type");
                }
                if (tangents->accessor.type != GltfType::Vec4) {
                    EXCEPTION("gltf object primitive tangents must be a Vec4 type");
                }
                if (skin && joints->accessor.type != GltfType::Vec4) {
                    EXCEPTION("gltf object primitive joints must be a Vec4 type");
                }
                if (skin && weights->accessor.type != GltfType::Vec4) {
                    EXCEPTION("gltf object primitive weights must be a Vec4 type");
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
                if (skin && joints->accessor.componentType != GltfComponentType::R8u) {
                    EXCEPTION("gltf object primitive joints must be component type of 8-bit int");
                }
                if (skin && weights->accessor.componentType != GltfComponentType::R32f) {
                    EXCEPTION("gltf object primitive weights must be component type of 32-bit float");
                }

                const auto count = positions->accessor.count;
                if (count != normals->accessor.count || count != texCoords->accessor.count ||
                    count != tangents->accessor.count) {
                    EXCEPTION("gltf object contains primitive which accessors has conflicting counts");
                }
                if (skin && (count != joints->accessor.count || count != weights->accessor.count)) {
                    EXCEPTION("gltf object contains primitive which accessors has conflicting counts");
                }

                // Update the bounding box
                bbMin = Vector3(positions->accessor.min);
                bbMax = Vector3(positions->accessor.max);

                // Update the bounding radius
                const auto span = positions->accessor.bufferView.getSpan();
                for (size_t i = 0; i < positions->accessor.count; i++) {
                    const auto* point = reinterpret_cast<const float*>(span.data()) + i * 3;
                    bbRadius = std::max(bbRadius, glm::length(Vector3{point[0], point[1], point[2]}));
                }

                const auto& partMaterial = part.material.value();

                auto& primitive = node.primitives.emplace_back();
                auto& material = materials.emplace_back();

                primitive.material = &material;
                material.uniform.baseColorFactor = partMaterial.baseColorFactor;
                material.uniform.emissiveFactor = partMaterial.emissiveFactor;
                material.uniform.metallicRoughnessFactor = partMaterial.metallicRoughnessFactor;
                material.uniform.normalFactor = Vector4{1.0f};
                material.uniform.ambientOcclusionFactor = Vector4{1.0f};

                // Only initialize textures if the Vulkan is present (client mode)
                if (vulkan) {
                    if (partMaterial.baseColorTexture) {
                        material.baseColorTexture = resolveTexture(partMaterial.baseColorTexture.value().getUri());
                    } else {
                        material.baseColorTexture = assetsManager.getDefaultTextures().baseColor;
                    }

                    if (partMaterial.normalTexture) {
                        material.normalTexture = resolveTexture(partMaterial.normalTexture.value().getUri());
                    } else {
                        material.normalTexture = assetsManager.getDefaultTextures().normal;
                    }

                    if (partMaterial.emissiveTexture) {
                        material.emissiveTexture = resolveTexture(partMaterial.emissiveTexture.value().getUri());
                    } else {
                        material.emissiveTexture = assetsManager.getDefaultTextures().emissive;
                    }

                    if (partMaterial.metallicRoughnessTexture) {
                        material.metallicRoughnessTexture =
                            resolveTexture(partMaterial.metallicRoughnessTexture.value().getUri());
                    } else {
                        material.metallicRoughnessTexture = assetsManager.getDefaultTextures().metallicRoughness;
                    }

                    if (partMaterial.ambientOcclusionTexture) {
                        material.ambientOcclusionTexture =
                            resolveTexture(partMaterial.ambientOcclusionTexture.value().getUri());
                    } else {
                        material.ambientOcclusionTexture = assetsManager.getDefaultTextures().ambient;
                    }

                    material.maskTexture = assetsManager.getDefaultTextures().mask;
                }

                switch (part.indices.value().componentType) {
                case GltfComponentType::R8: {
                    primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT8_EXT;
                    break;
                }
                case GltfComponentType::R8u: {
                    primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT8_EXT;
                    break;
                }
                case GltfComponentType::R16: {
                    primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT16;
                    break;
                }
                case GltfComponentType::R16u: {
                    primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT16;
                    break;
                }
                case GltfComponentType::R32u: {
                    primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
                    break;
                }
                default: {
                    EXCEPTION("Invalid index type");
                }
                }

                if (part.type != GltfPrimitiveType::Triangles) {
                    EXCEPTION("Invalid primitive type");
                }

                // Only initialize buffers if the Vulkan is present (client mode)
                if (vulkan) {
                    std::vector<float> vboData;
                    if (skin) {
                        vboData = GltfUtils::combine(positions->accessor,
                                                     normals->accessor,
                                                     texCoords->accessor,
                                                     tangents->accessor,
                                                     joints->accessor,
                                                     weights->accessor);
                    } else {
                        vboData = GltfUtils::combine(
                            positions->accessor, normals->accessor, texCoords->accessor, tangents->accessor);
                    }

                    const auto& iboData = part.indices->bufferView.getBuffer();

                    VulkanBuffer::CreateInfo bufferInfo{};
                    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    bufferInfo.size = vboData.size() * sizeof(float);
                    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
                    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                    primitive.mesh.vbo = vulkan->createBuffer(bufferInfo);
                    vulkan->copyDataToBuffer(primitive.mesh.vbo, vboData.data(), bufferInfo.size);

                    bufferInfo.size = iboData.size() * sizeof(uint8_t);
                    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                    primitive.mesh.ibo = vulkan->createBuffer(bufferInfo);
                    vulkan->copyDataToBuffer(primitive.mesh.ibo, iboData.data(), bufferInfo.size);

                    primitive.mesh.count = static_cast<uint32_t>(part.indices->count);

                    bufferInfo.size = iboData.size() * sizeof(uint8_t);
                    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                    material.ubo = vulkan->createBuffer(bufferInfo);
                    vulkan->copyDataToBuffer(material.ubo, &material.uniform, sizeof(Material::Uniform));
                }
            }
        }

        if (collisions && endsWith(collisions->name, "_CCX")) {
            if (!collisions->mesh) {
                EXCEPTION("gltf has collision node '{}' with no mesh", collisions->name);
            }
            if (collisions->mesh->primitives.empty()) {
                EXCEPTION("gltf has collision node '{}' with no mesh primitives", collisions->name);
            }
            const auto& part = collisions->mesh->primitives.front();
            const auto positions = findOne(
                part.attributes, [](const GltfAttribute& a) -> bool { return a.type == GltfAttributeType::Position; });

            if (!positions) {
                EXCEPTION("gltf has collision node '{}' with no vertices", collisions->name);
            }

            const auto span = positions->accessor.bufferView.getSpan();
            auto* points = reinterpret_cast<const float*>(span.data());
            auto shape = std::make_unique<btConvexHullShape>(
                points, static_cast<int>(positions->accessor.count), sizeof(float) * 3);
            collisionShape = std::move(shape);

            // Update the bounding box
            bbMin = Vector3(positions->accessor.min);
            bbMax = Vector3(positions->accessor.max);

            // Update the bounding radius
            bbRadius = 0.0f;
            for (size_t i = 0; i < positions->accessor.count; i++) {
                const auto* point = reinterpret_cast<const float*>(span.data()) + i * 3;
                bbRadius = std::max(bbRadius, glm::length(Vector3{point[0], point[1], point[2]}));
            }
        }

        if (!collisionShape) {
            collisionShape = std::make_unique<btSphereShape>(bbRadius);
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to load model: '{}'", getName());
    }
}

ModelPtr Model::from(const std::string& name) {
    return AssetsManager::getInstance().getModels().find(name);
}

void Model::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Model>("Model");
    cls["name"] = sol::readonly_property(&Model::getName);
    cls["radius"] = sol::readonly_property(&Model::getRadius);
}
