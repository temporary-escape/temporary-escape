#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Math/Matrix.hpp"

namespace Engine {
class ENGINE_API ShaderPbr : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;
    typedef VertexAttribute<1, Vector2> TextureCoordinates;

    enum Samplers : int {
        DepthTexture = 0,
        BaseColorTexture,
        EmissiveTexture,
        NormalTexture,
        MetallicRoughnessAmbientTexture,
        BrdfTexture,
        SkyboxIrradianceTexture,
        SkyboxPrefilterTexture,
        SSAOTexture,
    };

    enum Bindings : GLuint {
        Camera = 0,
        DirectionalLights = 1,
    };

    explicit ShaderPbr(const Config& config);
    virtual ~ShaderPbr() = default;

    ShaderPbr(const ShaderPbr& other) = delete;
    ShaderPbr(ShaderPbr&& other) = default;
    ShaderPbr& operator=(const ShaderPbr& other) = delete;
    ShaderPbr& operator=(ShaderPbr&& other) = default;

    void bindDepthTexture(const Texture& texture) const;
    void bindBaseColorTexture(const Texture& texture) const;
    void bindNormalTexture(const Texture& texture) const;
    void bindEmissiveTexture(const Texture& texture) const;
    void bindMetallicRoughnessAmbientTexture(const Texture& texture) const;
    void bindBrdfTexture(const Texture& texture) const;
    void bindSkyboxIrradianceTexture(const Texture& texture) const;
    void bindSkyboxPrefilterTexture(const Texture& texture) const;
    void bindSSAOTexture(const Texture& texture) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;
    void bindDirectionalLightsUniform(const VertexBuffer& ubo) const;

private:
    GLint cameraUniformIndex;
    GLint directionalLightsIndex;
    GLint ssaoUniformIndex;
};
} // namespace Engine
