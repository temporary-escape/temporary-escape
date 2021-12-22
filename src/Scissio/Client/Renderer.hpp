#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Framebuffer.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/Scene.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "../Shaders/ShaderSkybox.hpp"
#include "Client.hpp"
#include "SkyboxRenderer.hpp"
#include "WidgetDebugStats.hpp"

namespace Scissio {
class Renderer {
public:
    Renderer(Canvas2D& canvas, const Config& config, AssetManager& assetManager, Client& client);
    ~Renderer();

    void render(const Vector2i& viewport);

private:
    struct CameraUniform {
        Matrix4 transformationProjectionMatrix;
    };

    void createSkybox(uint64_t seed);
    void renderSceneSkybox(const Vector2i& viewport, Scene& scene);
    void renderScenePbr(const Vector2i& viewport, Scene& scene);
    void renderSceneForward(const Vector2i& viewport, Scene& scene);
    void renderComponent(ComponentSkybox& component);
    void renderComponent(ComponentModel& component);
    void blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target, FramebufferAttachment attachment);

    Canvas2D& canvas;
    GuiContext gui;
    Client& client;
    SkyboxRenderer skyboxRenderer;

    WidgetDebugStats widgetDebugStats;

    struct Shaders {
        ShaderSkybox skybox;
        ShaderModel model;
    } shaders;

    Camera camera;
    VertexBuffer cameraUbo;

    Texture2D defaultBaseColorTexture;
    Texture2D defaultNormalTexture;
    Texture2D defaultEmissiveTexture;
    Texture2D defaultMetallicRoughnessTexture;
    Texture2D defaultAmbientOcclusionTexture;
    Texture2D defaultSkyboxTexture;

    struct GBuffer {
        Vector2i size;
        Framebuffer fbo;
        bool fboInit{false};
        Texture2D fboDepth;
        Texture2D fboColorRoughness;
        Texture2D fboEmissiveMetallic;
        Texture2D fboNormalAmbient;
        Texture2D fboObjectId;
    } gBuffer;

    struct SkyboxData {
        Skybox textures;
        uint64_t seed{0};
        Mesh mesh{NO_CREATE};
    } skybox;
};
} // namespace Scissio
