#include "Imager.hpp"

#define CMP "Imager"

using namespace Engine;

Imager::Imager(const Config& config, AssetManager& assetManager, Renderer& renderer)
    : config(config), assetManager(assetManager), renderer(renderer) {
}

void Imager::createThumbnail(const AssetBlockPtr& block) {
    Log::i(CMP, "Creating thumbnails for block: '{}'", block->getName());

    Scene scene{};

    auto entity = std::make_shared<Entity>();
    entity->addComponent<ComponentSkybox>(Skybox::createOfColor(Color4{0.15f, 0.15f, 0.15f, 1.0f}));
    entity->scale(Vector3{1000.0f});
    scene.addEntity(entity);

    entity = std::make_shared<Entity>();
    entity->addComponent<ComponentDirectionalLight>(Color4{1.0f, 0.9f, 0.8f, 1.0f} * 3.0f);
    entity->translate(Vector3{-2.0f, 2.0f, 2.0f});
    scene.addEntity(entity);

    entity = std::make_shared<Entity>();
    auto cmp = entity->addComponent<ComponentCameraTurntable>();
    entity->translate(Vector3{0.0f, 0.0f, 0.0f});
    cmp->setProjection(48.0f);
    cmp->setZoom(2.0f);
    cmp->setRotation(Vector2{45.0f, -45.0f});
    scene.addEntity(entity);
    scene.setPrimaryCamera(entity);

    entity = std::make_shared<Entity>();
    auto grid = entity->addComponent<ComponentGrid>();
    scene.addEntity(entity);

    renderer.setEnableBackground(false);

    for (const auto shapeType : Shape::allTypes) {
        const auto& name = shapeTypeToFileName(shapeType);

        grid->setDirty(true);
        grid->insert(Vector3i{0}, block, 0, 0, shapeType);

        const auto viewport = Vector2i{256};
        render(scene, viewport);

        auto pixels = gBuffer.readPixels(viewport);
        auto image = assetManager.addToAtlas(viewport, pixels.get());
        assetManager.addImage(block->getMod(), fmt::format("image_{}_{}", block->getName(), name), image);
    }
}

void Imager::render(Scene& scene, const Vector2i& viewport) {
    gBuffer.resize(viewport);
    renderer.setViewport(viewport);
    renderer.setGBuffer(gBuffer);

    scene.update(0.1f);
    renderer.render(scene);
}
