#include "../Common.hpp"
#include <TemporaryEscape/Assets/AssetManager.hpp>
#include <TemporaryEscape/Scene/Scene.hpp>
#include <TemporaryEscape/Server/Bindings.hpp>
#include <wrenbind17/wrenbind17.hpp>

#define TAG "[Scene]"

using EntityRefCounter = RefCounter<Entity>;

struct EntityRefCounterHelper {
    static std::shared_ptr<Entity> create() {
        return std::make_shared<EntityRefCounter>(counter);
    }

    static inline size_t counter = 0;
};

struct SceneWithWrenFixture {
    SceneWithWrenFixture() : assetManager(config, canvas, textureCompressor), vm({tmp.value().string()}) {
        mod.path = tmp.value() / "base";
        mod.name = "base";
        expose(config, vm);
    }

    virtual ~SceneWithWrenFixture() {
        unexpose(config, vm);
    }

    Path writeTmpFile(const Path& path, const std::string& contents) {
        const auto fullPath = tmp.value() / path;
        Fs::create_directories(fullPath.parent_path());
        std::ofstream file(fullPath, std::ios::out);
        file << contents;
        return fullPath;
    }

    Config config;
    TmpDir tmp;
    Canvas2D canvas{NO_CREATE};
    TextureCompressor textureCompressor{NO_CREATE};
    Manifest mod;
    AssetManager assetManager;
    Scene scene;
    wrenbind17::VM vm;
};

TEST_CASE_METHOD(SceneWithWrenFixture, "Adding entity", TAG) {
    auto entity = std::make_shared<EntityRefCounter>(EntityRefCounterHelper::counter);
    scene.addEntity(entity);
    REQUIRE(entity.use_count() == 3);
}

TEST_CASE_METHOD(SceneWithWrenFixture, "Adding entity with script", TAG) {
    auto& m = vm.module("Engine");

    {
        auto& cls = m.klass<Entity>("EntityRefCounter");
        cls.funcStaticExt<&EntityRefCounterHelper::create>("create");
    }

    assetManager
        .addEntity(mod, writeTmpFile("base/entities/EntityFoo.wren", R"(
class EntityFoo {
    construct new (entity) {
        _entity = entity
    }

    getSomeMessage () {
        return "Hello World!"
    }
}
)"))
        ->load(assetManager);

    assetManager.runAllScripts(vm);

    vm.runFromSource("test", R"(
import "Engine" for EntityFactory, EntityRefCounter, Log

class SceneTest {
    construct new (scene) {
        var entity = EntityFactory.create("EntityFoo", EntityRefCounter.create())
        Log.d("Adding entity to scene")
        scene.addEntity(entity)
    }
}
)");

    REQUIRE(scene.getEntities().empty());
    REQUIRE(EntityRefCounterHelper::counter == 0);

    auto klass = vm.find("test", "SceneTest");
    REQUIRE(!!klass);

    auto constructor = klass.func("new(_)");
    REQUIRE(!!constructor);

    constructor(&scene);

    REQUIRE(scene.getEntities().size() == 1);
    REQUIRE(EntityRefCounterHelper::counter == 1);

    auto entity = scene.getEntities().front();

    auto r = entity->getComponent<ComponentScript>()->call("getSomeMessage()");
    REQUIRE(r.is<std::string>());
    REQUIRE(r.as<std::string>() == "Hello World!");

    // REQUIRE(entity.use_count() == 5);

    scene.removeEntity(entity);
    vm.gc();

    // REQUIRE(entity.use_count() == 2);

    REQUIRE(scene.getEntities().empty());
    REQUIRE(EntityRefCounterHelper::counter == 1);

    entity.reset();
    vm = wrenbind17::VM{};

    REQUIRE(EntityRefCounterHelper::counter == 0);
}
