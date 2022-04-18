#define CGLTF_IMPLEMENTATION
#include "GltfImporter.hpp"
#include "Base64.hpp"
#include "Exceptions.hpp"
#include <fstream>

using namespace Engine;

static std::string gltfErrorToString(const cgltf_result result) {
    switch (result) {
    case cgltf_result::cgltf_result_data_too_short: {
        return "data too short";
    }
    case cgltf_result::cgltf_result_file_not_found: {
        return "file not found";
    }
    case cgltf_result::cgltf_result_invalid_gltf: {
        return "invalid gltf";
    }
    case cgltf_result::cgltf_result_invalid_json: {
        return "invalid json";
    }
    case cgltf_result::cgltf_result_invalid_options: {
        return "invalid options";
    }
    case cgltf_result::cgltf_result_io_error: {
        return "input output error";
    }
    case cgltf_result::cgltf_result_unknown_format: {
        return "unknown format";
    }
    default: {
        return "unknown error";
    }
    }
}

GltfTexture::GltfTexture(const GltfData& data, cgltf_image* texture) : data(data), texture(texture) {
}

std::vector<uint8_t> GltfTexture::getRawBuffer(const Path& parentPath) const {
    if (texture->uri != nullptr) {
        if (std::strncmp(texture->uri, "data:", 5) == 0) {
            const auto uri = std::string(texture->uri);
            const auto pos = uri.find(";base64,");
            if (pos != std::string::npos) {
                return base64Decode(std::string_view(&uri[pos + 8], uri.size() - pos - 8));
            } else {
                EXCEPTION("Texture has invalid base64 format");
            }
        } else {
            std::fstream file(parentPath / Path(texture->uri), std::ios::in | std::ios::binary | std::ios::ate);
            if (!file) {
                EXCEPTION("Failed to open texture file: {}", texture->uri);
            }

            const auto size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> ret;
            ret.resize(size);
            file.read(reinterpret_cast<char*>(&ret[0]), size);

            return ret;
        }
    }

    else if (texture->buffer_view != nullptr) {
        const auto* src = reinterpret_cast<const char*>(texture->buffer_view->buffer->data);
        std::vector<uint8_t> ret;
        ret.resize(texture->buffer_view->size);
        std::memcpy(&ret[0], &src[texture->buffer_view->offset], texture->buffer_view->size);

        return ret;
    }

    else {
        EXCEPTION("Texture has no mime type or a buffer");
    }
}

std::string GltfTexture::getUri() const {
    if (texture->uri != nullptr) {
        if (std::strncmp(texture->uri, "data:", 5) == 0) {
            return "";
        } else {
            return texture->uri;
        }
    } else {
        return "";
    }
}

std::string GltfTexture::getMimeType() const {
    if (texture->uri != nullptr) {
        if (std::strncmp(texture->uri, "data:", 5) == 0) {
            const auto uri = std::string(texture->uri);
            const auto pos = uri.find(";base64,");
            if (pos != std::string::npos) {
                return uri.substr(5, pos - 5);
            } else {
                return "";
            }
        } else {
            auto ext = Path(texture->uri).extension();
            if (ext == ".png")
                return "image/png";
            if (ext == ".jpg")
                return "image/jpeg";
            if (ext == ".tga")
                return "image/tga";
            else
                return "";
        }
    } else {
        return texture->mime_type;
    }
}

GltfMaterial::GltfMaterial(const GltfData& data, cgltf_material* material) : data(data) {
    if (!material->has_pbr_metallic_roughness) {
        EXCEPTION("Material is not PBR metallic roughness");
    }

    baseColorFactor = Vector4(material->pbr_metallic_roughness.base_color_factor[0],
                              material->pbr_metallic_roughness.base_color_factor[1],
                              material->pbr_metallic_roughness.base_color_factor[2], 1.0f);
    emissiveFactor =
        Vector4(material->emissive_factor[0], material->emissive_factor[1], material->emissive_factor[2], 1.0f);

    metallicRoughnessFactor = Vector4(material->pbr_metallic_roughness.metallic_factor,
                                      material->pbr_metallic_roughness.roughness_factor, 0.0f, 0.0f);

    if (material->normal_texture.texture && material->normal_texture.texture->image) {
        normalTexture = {GltfTexture(data, material->normal_texture.texture->image)};
    }
    if (material->emissive_texture.texture && material->emissive_texture.texture->image) {
        emissiveTexture = {GltfTexture(data, material->emissive_texture.texture->image)};
    }
    if (material->pbr_metallic_roughness.base_color_texture.texture &&
        material->pbr_metallic_roughness.base_color_texture.texture->image) {
        baseColorTexture = {GltfTexture(data, material->pbr_metallic_roughness.base_color_texture.texture->image)};
    }
    if (material->occlusion_texture.texture && material->occlusion_texture.texture->image) {
        ambientOcclusionTexture = {GltfTexture(data, material->occlusion_texture.texture->image)};
    }
    if (material->pbr_metallic_roughness.metallic_roughness_texture.texture &&
        material->pbr_metallic_roughness.metallic_roughness_texture.texture->image) {
        metallicRoughnessTexture = {
            GltfTexture(data, material->pbr_metallic_roughness.metallic_roughness_texture.texture->image)};
    }
}

GltfBufferView::GltfBufferView(const GltfData& data, cgltf_buffer_view* view) : data(data), view(view) {
    type = static_cast<GltfBufferType>(view->type);
    size = view->size;
    offset = view->offset;
}

std::vector<uint8_t> GltfBufferView::getBuffer() const {
    std::vector<uint8_t> ret;
    ret.resize(size);

    const auto* src = reinterpret_cast<const uint8_t*>(view->buffer->data);
    std::memcpy(&ret[0], &src[view->offset], size);

    return ret;
}

Span<uint8_t> GltfBufferView::getSpan() const {
    const auto* src = reinterpret_cast<const uint8_t*>(view->buffer->data);
    return {&src[view->offset], size};
}

GltfAccessor::GltfAccessor(const GltfData& data, cgltf_accessor* accessor)
    : bufferView(data, accessor->buffer_view), data(data) {

    type = static_cast<GltfType>(accessor->type);
    componentType = static_cast<GltfComponentType>(accessor->component_type);
    min = Vector4(accessor->min[0], accessor->min[1], accessor->min[2], accessor->min[3]);
    max = Vector4(accessor->max[0], accessor->max[1], accessor->max[2], accessor->max[3]);
    count = accessor->count;
    stride = accessor->stride;
}

GltfAttribute::GltfAttribute(const GltfData& data, cgltf_attribute* attribute)
    : accessor(data, attribute->data), data(data) {

    type = static_cast<GltfAttributeType>(attribute->type);
}

GltfPrimitive::GltfPrimitive(const GltfData& data, cgltf_primitive* primitive) : data(data) {
    if (primitive->material) {
        material = GltfMaterial(data, primitive->material);
    }
    if (primitive->indices) {
        indices = GltfAccessor(data, primitive->indices);
    }
    type = static_cast<GltfPrimitiveType>(primitive->type);
    for (auto i = 0; i < static_cast<int>(primitive->attributes_count); i++) {
        attributes.emplace_back(data, &primitive->attributes[i]);
    }
}

GltfMesh::GltfMesh(const GltfData& data, cgltf_mesh* mesh) : data(data) {
    if (mesh->name) {
        name = mesh->name;
    }
    for (auto i = 0; i < static_cast<int>(mesh->primitives_count); i++) {
        primitives.emplace_back(data, &mesh->primitives[i]);
    }
}

GltfNode::GltfNode(const GltfData& data, cgltf_node* node) : data(data) {
    if (node->mesh) {
        mesh = GltfMesh(data, node->mesh);
    }
    if (node->name) {
        name = node->name;
    }
}

GltfImporter::GltfImporter(const Path& path) {
    cgltf_options options;
    std::memset(&options, 0, sizeof(options));
    cgltf_data* data = nullptr;
    const auto pathStr = path.string();
    cgltf_result result = cgltf_parse_file(&options, pathStr.c_str(), &data);
    if (result != cgltf_result_success) {
        EXCEPTION("Failed to open gltf file: {} error: {}", path.string(), gltfErrorToString(result));
    }

    result = cgltf_load_buffers(&options, data, pathStr.c_str());
    if (result != cgltf_result_success) {
        EXCEPTION("Failed to parse gltf buffers file: {} error: {}", path.string(), gltfErrorToString(result));
    }

    this->data = std::shared_ptr<cgltf_data>(data, [](cgltf_data* ptr) -> void { cgltf_free(ptr); });

    try {
        for (auto i = 0; i < static_cast<int>(this->data->materials_count); i++) {
            materials.emplace_back(this->data, &this->data->materials[i]);
        }

        for (auto i = 0; i < static_cast<int>(this->data->nodes_count); i++) {
            nodes.emplace_back(this->data, &this->data->nodes[i]);
        }
    } catch (std::exception& e) {
        EXCEPTION("Failed to parse gltf file: {}\n{}", path.string(), e.what());
    }
}

GltfImporter::~GltfImporter() = default;

std::vector<float> GltfUtils::combine(const GltfAccessor& position, const GltfAccessor& normals,
                                      const GltfAccessor& texCoords, const GltfAccessor& tangents) {
    const auto count = position.count;
    if (count != normals.count || count != texCoords.count || count != tangents.count) {
        EXCEPTION("accessor counts do not match");
    }

    if (count * 3 * sizeof(float) != position.bufferView.size) {
        EXCEPTION("accessor position buffer size does not match");
    }
    if (count * 3 * sizeof(float) != normals.bufferView.size) {
        EXCEPTION("accessor normals buffer size does not match");
    }
    if (count * 2 * sizeof(float) != texCoords.bufferView.size) {
        EXCEPTION("accessor texcoords buffer size does not match");
    }
    if (count * 4 * sizeof(float) != tangents.bufferView.size) {
        EXCEPTION("accessor tangents buffer size does not match");
    }

    std::vector<float> ret;
    ret.resize(position.count * 12);

    auto* dst = &ret[0];
    const auto positionRaw = position.bufferView.getBuffer();
    const auto normalsRaw = normals.bufferView.getBuffer();
    const auto texCoordsRaw = texCoords.bufferView.getBuffer();
    const auto tangentsRaw = tangents.bufferView.getBuffer();

    const auto* pos = reinterpret_cast<const float*>(&positionRaw[0]);
    const auto* nor = reinterpret_cast<const float*>(&normalsRaw[0]);
    const auto* tex = reinterpret_cast<const float*>(&texCoordsRaw[0]);
    const auto* tan = reinterpret_cast<const float*>(&tangentsRaw[0]);

    for (size_t i = 0; i < count; i++) {
        dst[0] = *pos++;
        dst[1] = *pos++;
        dst[2] = *pos++;

        dst[3] = *nor++;
        dst[4] = *nor++;
        dst[5] = *nor++;

        dst[6] = *tex++;
        dst[7] = *tex++;

        dst[8] = *tan++;
        dst[9] = *tan++;
        dst[10] = *tan++;
        dst[11] = *tan++;

        dst += 12;
    }

    return ret;
}
