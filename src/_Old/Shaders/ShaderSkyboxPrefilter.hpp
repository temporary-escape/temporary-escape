#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/TextureCubemap.hpp"
#include "../Math/Matrix.hpp"

namespace Engine {
class ENGINE_API ShaderSkyboxPrefilter : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;

    enum Samplers : int {
        SkyboxTexture = 0,
    };

    explicit ShaderSkyboxPrefilter(const Config& config);
    virtual ~ShaderSkyboxPrefilter() = default;

    ShaderSkyboxPrefilter(const ShaderSkyboxPrefilter& other) = delete;
    ShaderSkyboxPrefilter(ShaderSkyboxPrefilter&& other) = default;
    ShaderSkyboxPrefilter& operator=(const ShaderSkyboxPrefilter& other) = delete;
    ShaderSkyboxPrefilter& operator=(ShaderSkyboxPrefilter&& other) = default;

    void setProjectionMatrix(const Matrix4& matrix) const;
    void setViewMatrix(const Matrix4& matrix) const;
    void setRoughness(float value) const;
    void bindSkyboxTexture(const TextureCubemap& texture) const;

private:
    int projectionMatrixUniform;
    int viewMatrixUniform;
    int roughnessUniform;
};
} // namespace Engine
