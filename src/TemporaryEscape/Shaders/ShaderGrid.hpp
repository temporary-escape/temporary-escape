#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderGrid : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;
    using Normal = VertexAttribute<1, Vector3>;
    using TextureCoordinates = VertexAttribute<2, Vector2>;
    using Tangent = VertexAttribute<3, Vector4>;
    using Instances = VertexAttribute<4, Matrix4>;

    enum Samplers : GLuint {
        BaseColorTexture = 0,
        EmissiveTexture,
        NormalTexture,
        AmbientOcclusionTexture,
        MetallicRoughnessTexture,
    };

    enum Bindings : GLuint {
        Camera = 0,
        Material = 1,
    };

    explicit ShaderGrid(const Config& config);

    void setModelMatrix(const Matrix4& matrix) const;
    void setObjectId(const Vector2& color) const;
    void bindBaseColorTexture(const Texture& texture) const;
    void bindNormalTexture(const Texture& texture) const;
    void bindEmissiveTexture(const Texture& texture) const;
    void bindMetallicRoughnessTexture(const Texture& texture) const;
    void bindAmbientOcclusionTexture(const Texture& texture) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;
    void bindMaterialUniform(const VertexBuffer& ubo) const;

private:
    GLint cameraUniformIndex;
    GLint materialUniformIndex;
    GLint modelMatrixUniform;
    GLint objectIdUniform;
};
} // namespace Engine
