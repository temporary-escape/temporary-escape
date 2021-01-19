#pragma once

#include "../Assets/Primitive.hpp"
#include "../Config.hpp"
#include "../Graphics/Framebuffer.hpp"
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Shaders/ShaderGBufferView.hpp"
#include "../Shaders/ShaderGrid.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "../Shaders/ShaderPbr.hpp"
#include "../Shaders/ShaderSkybox.hpp"
#include "GBuffer.hpp"

namespace Scissio {
class Model;
struct Skybox;

class SCISSIO_API Renderer {
public:
    explicit Renderer(const Config& config);
    virtual ~Renderer() = default;

    // void beginFrame(const Vector2i& viewport, GBuffer& gBuffer);
    // void endFrame();

    // void blit(const Skybox& skybox, const GBuffer& gBuffer);
    // void endFrameDebug(int mode);
    // void renderGBuffer(int mode);

    void setCamera(const glm::mat4x4& viewMatrix);
    void setProjection(float fov = 70.0f);
    void setProjection(const Matrix4& projectionMatrix);
    void setViewport(const Vector2i& viewport);
    void renderPbr(const GBuffer& gBuffer, const Skybox& skybox);
    void renderSkybox(const TextureCubemap& texture);
    void renderModel(const Model& model, const Matrix4& transform);
    void renderGridModel(const std::list<Primitive>& primitives, const Matrix4& transform);

private:
    const Config& config;
    ShaderModel shaderModel;
    ShaderGrid shaderGrid;
    ShaderGBufferView shaderGBufferView;
    ShaderPbr shaderPbr;
    ShaderSkybox shaderSkybox;

    Mesh fullScreenQuad;
    Mesh skyboxMesh;

    Vector2i viewport;

    /*Framebuffer fbo;
    Texture2D fboDepth;
    Texture2D fboColorRoughness;
    Texture2D fboEmissiveMetallic;
    Texture2D fboNormalAmbient;*/

    Texture2D defaultBaseColor;
    Texture2D defaultNormal;
    Texture2D defaultEmissive;
    Texture2D defaultMetallicRoughness;
    Texture2D defaultAmbientOcclusion;

    Texture2D brdf;

    glm::mat4x4 projectionMatrix;
    glm::mat4x4 viewMatrix;
    glm::mat4x4 projectionViewMatrix;
};
} // namespace Scissio
