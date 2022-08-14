#include "Bindings.hpp"
#include "../Assets/Registry.hpp"
#include "../Scene/Scene.hpp"
#include <wrenbind17/wrenbind17.hpp>

using namespace Engine;

/*struct GlobalsHelper {
    explicit GlobalsHelper(const std::string& ptrStr) {
        vm = reinterpret_cast<wrenbind17::VM*>(std::stoull(ptrStr));
        const auto it = vms.find(vm);
        if (it == vms.end()) {
            EXCEPTION("VM not exposed correctly!");
        }
        assetManager = &AssetManager::singleton();
    }

    AssetManager* assetManager;
    wrenbind17::VM* vm;

    [[nodiscard]] wrenbind17::Variable assetEntityToClass(const std::string& name) const {
        const auto asset = assetManager->find<AssetEntity>(name);
        auto klass = vm->find(asset->getModuleName(), asset->getClassName());
        if (!klass) {
            EXCEPTION("No class named '{}' in module '{}'", asset->getClassName(), asset->getModuleName());
        }
        return klass;
    }

    static inline std::set<wrenbind17::VM*> vms;
};*/

/*struct LogHelper {
    static void i(const std::string& msg) {
        Log::i("Wren", msg);
    }

    static void w(const std::string& msg) {
        Log::w("Wren", msg);
    }

    static void e(const std::string& msg) {
        Log::e("Wren", msg);
    }

    static void d(const std::string& msg) {
        Log::d("Wren", msg);
    }
};*/

/*struct EntityHelper {
    static void addComponentScript(Entity& self, wrenbind17::Variable script) {
        auto cmp = self.addComponent<ComponentScript>();
        cmp->setScript(std::move(script));
    }

    static wrenbind17::Variable getScript(Entity& self) {
        auto cmp = self.getComponent<ComponentScript>();
        return cmp->getScript();
    }
};*/

void Engine::unexpose(const Config& config, wrenbind17::VM& vm) {
    // GlobalsHelper::vms.erase(&vm);
}

void Engine::expose(const Config& config, wrenbind17::VM& vm) {
    /*GlobalsHelper::vms.insert(&vm);

    auto& m = vm.module("Engine");

    {
        auto& cls = m.klass<LogHelper>("Log");
        cls.funcStatic<&LogHelper::i>("i");
        cls.funcStatic<&LogHelper::w>("w");
        cls.funcStatic<&LogHelper::e>("e");
        cls.funcStatic<&LogHelper::d>("d");
    }

    {
        auto& cls = m.klass<GlobalsHelper>("GlobalsHelper");
        cls.ctor<const std::string&>();
        cls.varReadonly<&GlobalsHelper::assetManager>("assetManager");
        cls.func<&GlobalsHelper::assetEntityToClass>("assetEntityToClass");
    }

    {
        auto& cls = m.klass<AssetManager>("AssetManager");
        cls.func<&AssetManager::find<AssetModel>>("findModel");
        cls.func<&AssetManager::find<AssetBlock>>("findBlock");
    }

    {
        auto& cls = m.klass<AssetModel>("AssetModel");
        cls.propReadonly<&Asset::getName>("name");
    }

    {
        auto& cls = m.klass<AssetBlock>("AssetBlock");
        cls.propReadonly<&Asset::getName>("name");
        cls.propReadonly<&AssetBlock::getTitle>("title");
    }

    { auto& cls = m.klass<AssetEntity>("AssetEntity"); }

    {
        auto& cls = m.klass<Entity>("Entity");
        cls.ctor<>();
        cls.funcExt<&EntityHelper::addComponentScript>("addComponentScript");
        cls.propReadonlyExt<&EntityHelper::getScript>("script");
    }

    {
        auto& cls = m.klass<Scene>("Scene");
        cls.func<&Scene::addEntity>("addEntity");
    }

    m.append(fmt::format("var Globals = GlobalsHelper.new(\"{}\")\n", std::to_string(reinterpret_cast<uint64_t>(&vm))));

    m.append(R"(
class EntityFactory {
    static create (name) {
        return EntityFactory.create(name, Entity.new())
    }

    static create (name, entity) {
        var klass = Globals.assetEntityToClass(name)
        var script = klass.new(entity)
        entity.addComponentScript(script)
        return entity
    }
}
)");*/
}
