#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Framebuffer.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/Scene.hpp"
#include "../Shaders/ShaderBrdf.hpp"
#include "../Shaders/ShaderModel.hpp"
#include "../Shaders/ShaderPbr.hpp"
#include "../Shaders/ShaderPlanetAtmosphere.hpp"
#include "../Shaders/ShaderPlanetSurface.hpp"
#include "../Shaders/ShaderSkybox.hpp"
#include "SkyboxRenderer.hpp"

namespace Scissio {
class SCISSIO_API Renderer {
public:
    static constexpr inline size_t maxDirectionalLights = 4;

    Renderer(const Config& config, Canvas2D& canvas, AssetManager& assetManager);
    ~Renderer();

    void render(const Vector2i& viewport, Scene& scene);
    void reloadShaders();

private:
    struct CameraUniform {
        Matrix4 transformationProjectionMatrix;
        Matrix4 viewProjectionInverseMatrix;
        Vector2i viewport;
        char padding0[sizeof(int) * 2];
        Vector3 eyesPos;
        char padding1[sizeof(float) * 1];
    };

    struct DirectionalLightsUniform {
        Vector4 colors[maxDirectionalLights];
        Vector4 directions[maxDirectionalLights];
        int count{0};
    };

    void createSkybox(uint64_t seed);
    void createBrdfTexture();
    void createFullScreenMesh();
    void createPlanetMesh();
    void updateLights(Scene& scene);
    Camera* getPrimaryCamera(Scene& scene);
    void updateCameras(const Vector2i& viewport, Scene& scene);
    void renderPbr();
    void renderCanvas(const Vector2i& viewport, Scene& scene);
    void renderSceneBackground(const Vector2i& viewport, Scene& scene);
    void renderScenePbr(const Vector2i& viewport, Scene& scene);
    void renderSceneForward(const Vector2i& viewport, Scene& scene);
    void renderComponentSkybox(ComponentSkybox& component);
    void renderComponentPlanetSurface(ComponentPlanet& component);
    void renderComponentPlanetAtmosphere(ComponentPlanet& component);
    void renderComponentModel(ComponentModel& component);
    void renderComponentCanvasImage(const Camera& camera, ComponentCanvasImage& component);
    void renderComponentCanvasLines(const Camera& camera, ComponentCanvasLines& component);
    void renderComponentCanvasLabel(const Camera& camera, ComponentCanvasLabel& component);
    void blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target, FramebufferAttachment attachment);

    const Config& config;
    Canvas2D& canvas;
    // GuiContext gui;
    // Client& client;
    SkyboxRenderer skyboxRenderer;

    // WidgetDebugStats widgetDebugStats;

    struct Shaders {
        ShaderSkybox skybox;
        ShaderModel model;
        ShaderBrdf brdf;
        ShaderPbr pbr;
        ShaderPlanetSurface planetSurface;
        ShaderPlanetAtmosphere planetAtmosphere;
    } shaders;

    VertexBuffer cameraUbo{NO_CREATE};
    VertexBuffer cameraZeroPosUbo{NO_CREATE};
    VertexBuffer directionalLightsUbo{NO_CREATE};

    Mesh fullScreenMesh{NO_CREATE};

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
        Texture2D fboColorAlpha;
        Texture2D fboEmissive;
        Texture2D fboNormal;
        Texture2D fboMetallicRoughnessAmbient;
    } gBuffer;

    struct SkyboxData {
        Skybox textures;
        uint64_t seed{0};
        Mesh mesh{NO_CREATE};
    } skybox;

    struct BrdfData {
        Texture2D texture{NO_CREATE};
    } brdf;

    struct PlanetData {
        Mesh mesh{NO_CREATE};
        AssetTexturePtr surfaceTexture;
    } planet;
};
} // namespace Scissio
