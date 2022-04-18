#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Math/Matrix.hpp"

namespace Engine {
class ENGINE_API ShaderSSAO : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;
    typedef VertexAttribute<1, Vector2> TextureCoordinates;

    enum Samplers : int {
        DepthTexture = 0,
        NormalTexture,
        NoiseTexture,
    };

    enum Bindings : GLuint {
        Camera = 0,
    };

    explicit ShaderSSAO(const Config& config);
    virtual ~ShaderSSAO() = default;

    ShaderSSAO(const ShaderSSAO& other) = delete;
    ShaderSSAO(ShaderSSAO&& other) = default;
    ShaderSSAO& operator=(const ShaderSSAO& other) = delete;
    ShaderSSAO& operator=(ShaderSSAO&& other) = default;

    void bindDepthTexture(const Texture& texture) const;
    void bindNormalTexture(const Texture& texture) const;
    void bindNoiseTexture(const Texture& texture) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;
    void setNoiseScale(const Vector2& value) const;
    void setSamples(const std::vector<Vector3>& samples) const;

private:
    GLint cameraUniformIndex;
    GLint samplesUniform;
    GLint noiseScaleUniform;
};
} // namespace Engine
