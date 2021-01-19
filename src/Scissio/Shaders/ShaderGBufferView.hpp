#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"

#include <glm/mat4x4.hpp>

namespace Scissio {
class SCISSIO_API ShaderGBufferView : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;

    enum : int {
        DepthTexture = 0,
        BaseColorRoughnessTexture,
        NormalMetallicTexture,
        EmissiveAmbientTexture,
    };

    explicit ShaderGBufferView(const Config& config);
    virtual ~ShaderGBufferView() = default;

    void bindDepthTexture(const Texture& texture);
    void bindBaseColorRoughnessTexture(const Texture& texture);
    void bindNormalMetallicTexture(const Texture& texture);
    void bindEmissiveAmbientTexture(const Texture& texture);
    void setViewport(const Vector2i& viewport);
    void setMode(int mode);

private:
    int viewportUniform;
    int modeUniform;
};
} // namespace Scissio
