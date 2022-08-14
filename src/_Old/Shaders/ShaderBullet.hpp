#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderBullet : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;
    using Direction = VertexAttribute<1, Vector3>;
    using Color = VertexAttribute<2, Color4>;
    using Time = VertexAttribute<3, float>;
    using Speed = VertexAttribute<4, float>;

    enum Bindings : GLuint {
        Camera = 0,
    };

    explicit ShaderBullet(const Config& config);

    ShaderBullet(const ShaderBullet& other) = delete;
    ShaderBullet(ShaderBullet&& other) = default;
    ShaderBullet& operator=(const ShaderBullet& other) = delete;
    ShaderBullet& operator=(ShaderBullet&& other) = default;

    void bindCameraUniform(const VertexBuffer& ubo) const;

private:
    GLint cameraUniformIndex;
};
} // namespace Engine
