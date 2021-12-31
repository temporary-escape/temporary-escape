#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Scissio {
class ShaderPlanetSurface : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;
    using Normal = VertexAttribute<1, Vector3>;

    enum Bindings : GLuint {
        Camera = 0,
        DirectionalLights = 1,
    };

    enum Samplers : GLuint {
        SurfaceTexture = 0,
    };

    explicit ShaderPlanetSurface(const Config& config);

    void setModelMatrix(const Matrix4& matrix) const;
    void setNormalMatrix(const Matrix3& matrix) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;
    void bindDirectionalLightsUniform(const VertexBuffer& ubo) const;
    void bindSurfaceTexture(const Texture& texture) const;

private:
    GLint cameraUniformIndex;
    GLint directionalLightsIndex;
    GLint normalMatrixUniform;
    GLint modelMatrixUniform;
};
} // namespace Scissio
