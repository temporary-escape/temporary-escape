#pragma once

#include "../Assets/AssetShape.hpp"
#include "../Assets/SkyboxRenderer.hpp"
#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Framebuffer.hpp"
#include "../Graphics/MeshPrimitives.hpp"
#include "../Scene/Camera.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/Scene.hpp"
#include "GBuffer.hpp"
#include "Shaders.hpp"

namespace Engine {
class ENGINE_API Renderer {
public:
    struct DirectionalLight {
        Vector4 color{};
        Vector3 direction{};
    };

    explicit Renderer(const Config& config, const Shaders& shaders, SkyboxRenderer& skyboxRenderer,
                      Grid::Builder& gridBuilder);

    void setGBuffer(GBuffer& gBuffer) {
        state.gBuffer = &gBuffer;
    }

    void setViewport(const Vector2i& viewport) {
        state.viewport = viewport;
    }

    void setEnableBackground(bool value) {
        state.renderBackground = value;
    }

    void render(Scene& scene);

private:
    void setCamera(Camera& camera);
    void setDirectionalLights(const std::vector<DirectionalLight>& lights);
    void createBrdfTexture();
    void renderSceneDeffered(Scene& scene);
    void renderSceneBackground(Scene& scene);
    void renderSceneForward(Scene& scene);
    void renderPbrBuffer();
    void renderSSAO();
    void renderFXAA();
    void renderBloomExtract();
    void renderBloomBlurVertical();
    void renderBloomBlurHorizontal();
    void renderBloomCombine();
    void renderDebugNormals(Scene& scene);
    void renderSkybox(const TextureCubemap& cubemap, const Matrix4& transform);
    void renderModel(const AssetModelPtr& model, const Matrix4& transform);
    void renderComponentModel(ComponentModel& component);
    void renderComponentGrid(ComponentGrid& component);
    void renderComponentTurret(ComponentTurret& component);
    void renderComponentParticleEmitter(ComponentParticleEmitter& component);
    void renderDebugNormalsModel(ComponentModel& component);
    void renderDebugNormalsGrid(ComponentGrid& component);
    Texture2D generateSSAONoise();
    std::vector<Vector3> generateSSAOSamples();

    static constexpr inline size_t maxDirectionalLights = 4;

    const Config& config;
    const Shaders& shaders;
    SkyboxRenderer& skyboxRenderer;
    Grid::Builder& gridBuilder;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    float time;

    struct CameraUniform {
        Matrix4 transformationProjectionMatrix;
        Matrix4 viewProjectionInverseMatrix;
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
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

    struct {
        Mesh fullScreenQuad = createFullScreenQuad();
        Mesh skybox = createSkyboxMesh();
        Mesh planet = createPlanetMesh();
    } meshes;

    struct {
        Texture2D baseColorTexture{NO_CREATE};
        Texture2D normalTexture{NO_CREATE};
        Texture2D emissiveTexture{NO_CREATE};
        Texture2D metallicRoughnessTexture{NO_CREATE};
        Texture2D ambientOcclusionTexture{NO_CREATE};
        TextureCubemap skyboxTexture{NO_CREATE};
    } defaultTextures;

    struct {
        Texture2D texture{NO_CREATE};
    } brdf;

    struct {
        GBuffer* gBuffer{nullptr};
        Vector2i viewport{};
        VertexBuffer cameraUbo{NO_CREATE};
        VertexBuffer cameraZeroPosUbo{NO_CREATE};
        VertexBuffer directionalLightsUbo{NO_CREATE};
        const Skybox* skybox{nullptr};
        Matrix4 cameraViewMatrix;
        bool renderBackground{true};
    } state;

    struct {
        VertexArray vao{NO_CREATE};
    } particleEmitter;

    struct {
        std::vector<Vector3> samples;
        Texture2D noise;
    } ssao;

    // PostProcessingFXAA fxaa;
    // PostProcessingSSAO ssao;

    /*static constexpr inline size_t ssaoSamplesNum = 8;
    static constexpr inline size_t ssaoNoiseNum = 4;
    static constexpr inline size_t maxDirectionalLights = 4;

    Renderer(const Config& config, Canvas2D& canvas, AssetManager& assetManager);
    ~Renderer();

    void render(const Vector2i& viewport, Scene& scene);
    Camera* getPrimaryCamera(Scene& scene);
    void reloadShaders();
    EntityPtr queryEntityAtPos(Scene& scene, const Vector2i& pos);

private:
    struct CameraUniform {
        Matrix4 transformationProjectionMatrix;
        Matrix4 viewProjectionInverseMatrix;
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
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

    struct SSAOUniform {
        Vector4 samples[ssaoSamplesNum];
        Vector4 noise[ssaoNoiseNum];
    };

    void createSkybox(uint64_t seed);
    void createBrdfTexture();
    void createSSAOUbo();
    void createFullScreenMesh();
    void createPlanetMesh();
    void updateLights(Scene& scene);
    void updateBullets(Scene& scene);
    void updateCameras(const Vector2i& viewport, Scene& scene);
    void renderPbr();
    void renderBullets();
    void renderCanvas(const Vector2i& viewport, Scene& scene);
    void renderSceneBackground(const Vector2i& viewport, Scene& scene);
    void renderScenePbr(const Vector2i& viewport, Scene& scene);
    void renderSceneForward(const Vector2i& viewport, Scene& scene);
    void applyFxaa(const Vector2i& viewport);
    void renderComponentSkybox(ComponentSkybox& component);
    void renderSkybox(const Matrix4& modelMatrix, const TextureCubemap& cubemap);
    void renderComponentPlanetSurface(ComponentPlanet& component);
    void renderComponentPlanetAtmosphere(ComponentPlanet& component);
    void renderComponentModel(ComponentModel& component);
    void renderModel(const AssetModelPtr& model, const Matrix4& transform);
    void renderComponentTurret(ComponentTurret& component);
    void renderComponentParticleEmitter(ComponentParticleEmitter& component);
    void renderComponentPointCloud(ComponentPointCloud& component);
    void renderComponentLines(ComponentLines& component);
    // void renderComponentCanvasImage(const Camera& camera, ComponentPointCloud& component);
    // void renderComponentCanvasLines(const Camera& camera, ComponentLines& component);
    void renderComponentText(const Camera& camera, ComponentText& component);
    void blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target, FramebufferAttachment attachment,
              BufferBit bufferBit);

    const Config& config;
    Canvas2D& canvas;
    SkyboxRenderer skyboxRenderer;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    float time;

    struct Shaders {
        ShaderSkybox skybox;
        ShaderModel model;
        ShaderGrid grid;
        ShaderBrdf brdf;
        ShaderPbr pbr;
        ShaderPlanetSurface planetSurface;
        ShaderPlanetAtmosphere planetAtmosphere;
        ShaderParticleEmitter particleEmitter;
        ShaderBullet bullet;
        ShaderFXAA fxaa;
        ShaderPointCloud pointCloud;
        ShaderLines lines;
    } shaders;

    Matrix4 cameraViewMatrix;
    VertexBuffer cameraUbo{NO_CREATE};
    VertexBuffer cameraZeroPosUbo{NO_CREATE};
    VertexBuffer directionalLightsUbo{NO_CREATE};
    VertexBuffer ssaoUbo{NO_CREATE};

    Mesh fullScreenMesh{NO_CREATE};

    Texture2D defaultBaseColorTexture;
    Texture2D defaultNormalTexture;
    Texture2D defaultEmissiveTexture;
    Texture2D defaultMetallicRoughnessTexture;
    Texture2D defaultAmbientOcclusionTexture;
    TextureCubemap defaultSkyboxTexture;

    struct GBuffer {
        Vector2i size;
        Framebuffer fbo;
        bool fboInit{false};
        Texture2D fboDepth;
        Texture2D fboColorAlpha;
        Texture2D fboEmissive;
        Texture2D fboNormal;
        Texture2D fboMetallicRoughnessAmbient;
        Texture2D fboObjectId;
    } gBuffer;

    struct PostProcessing {
        Vector2i size;
        bool fboInit{false};
        Texture2D fboDepth;

        Framebuffer fbo[2];
        Texture2D fboTexture[2];

        size_t inputIdx{0};
        size_t outputIdx{1};
    } postProcessing;

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

    struct ParticleEmitterData {
        VertexArray vao{NO_CREATE};
    } particleEmitter;

    struct BulletData {
        Mesh mesh{NO_CREATE};
        VertexBuffer vbo{NO_CREATE};
        size_t size{0};
    } bullets;*/
};
} // namespace Engine
