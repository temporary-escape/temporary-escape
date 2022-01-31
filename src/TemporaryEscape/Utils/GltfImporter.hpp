#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "Path.hpp"

#include <optional>
#include <string>
#include <vector>

extern "C" {
#include <cgltf.h>
}

namespace Engine {
typedef std::shared_ptr<cgltf_data> GltfData;

enum class GltfType {
    Invalid = cgltf_type_invalid,
    Scalar = cgltf_type_scalar,
    Vec2 = cgltf_type_vec2,
    Vec3 = cgltf_type_vec3,
    Vec4 = cgltf_type_vec4,
    Mat2 = cgltf_type_mat2,
    Mat3 = cgltf_type_mat3,
    Mat4 = cgltf_type_mat4
};

enum class GltfPrimitiveType {
    Points = cgltf_primitive_type_points,
    Lines = cgltf_primitive_type_lines,
    LineLoop = cgltf_primitive_type_line_loop,
    LineStrip = cgltf_primitive_type_line_strip,
    Triangles = cgltf_primitive_type_triangles,
    TriangleStrip = cgltf_primitive_type_triangle_strip,
    TriangleFan = cgltf_primitive_type_triangle_fan,
};

enum class GltfAttributeType {
    Invalid = cgltf_attribute_type_invalid,
    Position = cgltf_attribute_type_position,
    Normal = cgltf_attribute_type_normal,
    Tangent = cgltf_attribute_type_tangent,
    TexCoord = cgltf_attribute_type_texcoord,
    Color = cgltf_attribute_type_color,
    Joints = cgltf_attribute_type_joints,
    Weights = cgltf_attribute_type_weights,
};

enum class GltfComponentType {
    Invalid = cgltf_component_type_invalid,
    R8 = cgltf_component_type_r_8,
    R8u = cgltf_component_type_r_8u,
    R16 = cgltf_component_type_r_16,
    R16u = cgltf_component_type_r_16u,
    R32u = cgltf_component_type_r_32u,
    R32f = cgltf_component_type_r_32f,
};

enum class GltfBufferType {
    Invalid = cgltf_buffer_view_type_invalid,
    Indices = cgltf_buffer_view_type_indices,
    Vertices = cgltf_buffer_view_type_vertices,
};

class ENGINE_API GltfTexture {
public:
    explicit GltfTexture(const GltfData& data, cgltf_image* texture);

    std::vector<uint8_t> getRawBuffer(const Path& parentPath) const;
    std::string getMimeType() const;
    std::string getUri() const;

private:
    GltfData data;
    cgltf_image* texture;
};

class ENGINE_API GltfMaterial {
public:
    explicit GltfMaterial(const GltfData& data, cgltf_material* material);

    Vector4 emissiveFactor;
    Vector4 baseColorFactor;
    Vector4 metallicRoughnessFactor;
    std::optional<GltfTexture> baseColorTexture;
    std::optional<GltfTexture> emissiveTexture;
    std::optional<GltfTexture> normalTexture;
    std::optional<GltfTexture> ambientOcclusionTexture;
    std::optional<GltfTexture> metallicRoughnessTexture;

private:
    GltfData data;
};

class ENGINE_API GltfBufferView {
public:
    explicit GltfBufferView(const GltfData& data, cgltf_buffer_view* view);

    GltfBufferType type;
    size_t size;
    size_t offset;

    std::vector<uint8_t> getBuffer() const;

private:
    GltfData data;
    cgltf_buffer_view* view;
};

class ENGINE_API GltfAccessor {
public:
    explicit GltfAccessor(const GltfData& data, cgltf_accessor* accessor);

    GltfType type;
    GltfComponentType componentType;
    Vector4 min;
    Vector4 max;
    size_t count;
    size_t stride;
    GltfBufferView bufferView;

private:
    GltfData data;
};

class ENGINE_API GltfAttribute {
public:
    explicit GltfAttribute(const GltfData& data, cgltf_attribute* attribute);

    GltfAttributeType type;
    GltfAccessor accessor;

private:
    GltfData data;
};

class ENGINE_API GltfPrimitive {
public:
    explicit GltfPrimitive(const GltfData& data, cgltf_primitive* primitive);

    std::vector<GltfAttribute> attributes;
    GltfPrimitiveType type;
    std::optional<GltfMaterial> material;
    std::optional<GltfAccessor> indices;

private:
    GltfData data;
};

class ENGINE_API GltfMesh {
public:
    explicit GltfMesh(const GltfData& data, cgltf_mesh* mesh);

    std::string name;
    std::vector<GltfPrimitive> primitives;

private:
    GltfData data;
};

class ENGINE_API GltfNode {
public:
    explicit GltfNode(const GltfData& data, cgltf_node* node);

    std::string name;
    std::optional<GltfMesh> mesh;

private:
    GltfData data;
};

class ENGINE_API GltfImporter {
public:
    explicit GltfImporter(const Path& path);
    virtual ~GltfImporter();

    const std::vector<GltfMaterial>& getMaterials() const {
        return materials;
    }

    const std::vector<GltfNode>& getNodes() const {
        return nodes;
    }

private:
    GltfData data;
    std::vector<GltfMaterial> materials;
    std::vector<GltfNode> nodes;
};

class ENGINE_API GltfUtils {
public:
    static std::vector<float> combine(const GltfAccessor& position, const GltfAccessor& normals,
                                      const GltfAccessor& texCoords, const GltfAccessor& tangents);
};
} // namespace Engine
