#pragma once

#include "../Assets/Primitive.hpp"
#include "../Config.hpp"
#include "../Graphics/Framebuffer.hpp"
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Scene/Object.hpp"
#include "../Shaders/ShaderGBufferView.hpp"
#include "../Shaders/ShaderGrid.hpp"
#include "../Shaders/ShaderLines.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "../Shaders/ShaderParticleEmitter.hpp"
#include "../Shaders/ShaderPbrBuffer.hpp"
#include "../Shaders/ShaderPointCloud.hpp"
#include "../Shaders/ShaderSkybox.hpp"
#include "../Shaders/ShaderWireframe.hpp"
#include "FBuffer.hpp"
#include "GBuffer.hpp"

namespace Scissio {
class Scene;
struct Skybox;

struct RenderOptions {
    bool withSkybox{true};
};

class SCISSIO_API Renderer {
public:
    explicit Renderer(const Config& config);
    ~Renderer() = default;

    void setView(const glm::mat4x4& viewMatrix);
    void setProjection(float fov = 70.0f);
    void setProjection(const Matrix4& projectionMatrix);
    void setViewport(const Vector2i& viewport);
    void setSkybox(const Skybox& skybox);
    void setGBuffer(GBuffer& gbuffer);
    void setFBuffer(FBuffer& fbuffer);
    void setQueryPos(const Vector2i& queryPos);
    std::optional<Object*> render(Scene& scene, const RenderOptions& options = RenderOptions{});

private:
    void renderGBuffer();

    ShaderModel shaderModel;
    ShaderGrid shaderGrid;
    ShaderGBufferView shaderGBufferView;
    ShaderPbrBuffer shaderPbr;
    ShaderSkybox shaderSkybox;
    ShaderPointCloud shaderPointCloud;
    ShaderLines shaderLines;
    ShaderWireframe shaderWireframe;
    ShaderParticleEmitter shaderParticleEmitter;

    Texture2D brdf;
    Vector2i viewport;
    bool viewportChanged;
    Vector2i queryPos{};

    const Skybox* skybox;
    GBuffer* gbuffer;
    FBuffer* fbuffer;

    Mesh fullScreenQuad;
    Mesh skyboxMesh;

    glm::mat4x4 projectionMatrix{};
    glm::mat4x4 viewMatrix{};
    glm::mat4x4 projectionViewMatrix{};
};
}; // namespace Scissio
