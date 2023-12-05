#include "Lua.hpp"
#include "../Math/DelaunayTriangulation.hpp"
#include "../Math/FloodFill.hpp"
#include "../Math/GalaxyDistribution.hpp"
#include "../Math/MinimumSpanningTree.hpp"
#include "../Math/Random.hpp"
#include "../Math/VolumeOccupancyTester.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "../Utils/NameGenerator.hpp"
#include "../Utils/Path.hpp"
#include "../Utils/Random.hpp"
#include "../Utils/StringUtils.hpp"
#include "Server.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static std::unordered_map<lua_State*, Lua::Data*> instances;

static auto logger = createLogger(LOG_FILENAME);

static void backtrace(std::stringstream& out, const std::exception& e) {
    out << fmt::format("{}\n", e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& ex) {
        backtrace(out, ex);
    } catch (...) {
    }
}

static int exceptionHandler(lua_State* L, sol::optional<const std::exception&> maybeException,
                            sol::string_view description) {
    logger.error("Lua exception");
    // const char* message = lua_tostring(L, -1);
    // luaL_traceback(L, L, message, 1);
    if (!description.empty()) {
        logger.error("{}", description);
    }

    if (maybeException) {
        std::stringstream ss;
        backtrace(ss, *maybeException);
        return sol::stack::push(L, ss.str());
    }
    return sol::stack::push(L, description);
}

static void myPanic(sol::optional<std::string> maybeMsg) {
    if (maybeMsg) {
        logger.error("Lua panic: {}", *maybeMsg);
    }
    logger.error("Lua panic: unknown error");
}

Lua::Binder::Binder(std::function<void(sol::table&)> fn, std::string_view name) : fn{std::move(fn)}, name{name} {
    Lua::registerBinder(*this);
}

void Lua::Binder::setup(sol::table& m) {
    try {
        fn(m);
    } catch (...) {
        EXCEPTION_NESTED("Failed to setup lua bindings for: {}", name);
    }
}

struct Lua::Data {
    sol::state state{sol::c_call<decltype(&myPanic), myPanic>};
    sol::table engine;
};

static sol::table requireEngine(sol::this_state state) {
    logger.info("Require engine lua state: {}", reinterpret_cast<uint64_t>(state.lua_state()));

    auto& self = instances.at(state.lua_state());
    return self->engine;
}

class Lua::EventHandler : EventBus::Listener {
public:
    explicit EventHandler(EventBus& eventBus) : EventBus::Listener{eventBus} {
    }

    static void bind(Lua& lua) {
        auto& m = lua.root();

        auto cls = m.new_usertype<EventHandler>("EventHandler");
        cls["add_handler"] = [](EventHandler& self, std::string name, sol::function fn) {
            const auto* fnId = fn.pointer();

            self.handlerMap[fnId] = self.addHandler(std::move(name), [fn](const EventData& data) {
                sol::protected_function_result result = fn(sol::as_table(data));
                if (!result.valid()) {
                    sol::error err = result;
                    EXCEPTION("{}", err.what());
                }
            });
        };
        cls["remove_handler"] = [](EventHandler& self, std::string name, sol::function fn) {
            const auto* fnId = fn.pointer();
            const auto it = self.handlerMap.find(fnId);
            if (it != self.handlerMap.end()) {
                self.removeHandler(name, it->second);
                self.handlerMap.erase(it);
            }
        };
        cls["enqueue"] = [](EventHandler& self, std::string name, sol::as_table_t<EventData> data) {
            self.enqueue(std::move(name), data.value());
        };
    }

private:
    std::unordered_map<const void*, EventBus::Handle> handlerMap;
};

Lua::Lua(const Config& config, EventBus& eventBus) : config{config}, data{std::make_unique<Data>()} {
    logger.info("Creating lua...");

    // Common Lua libraries
    data->state.open_libraries(
        sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::debug, sol::lib::math, sol::lib::table);

    // Custom exception handler to handle nested exceptions
    data->state.set_exception_handler(&exceptionHandler);

    // Global variables for easy data sharing across mods
    data->state["globals"] = data->state.create_table();

    // The "engine" module.
    data->engine = data->state.create_table();

    // Remember out instance
    instances.insert(std::make_pair(data->state.lua_state(), data.get()));

    // The "engine" module will be exposed via the require() function
    data->state.require("engine", sol::c_call<decltype(&requireEngine), &requireEngine>, false);

    logger.info("Created lua state: {}", reinterpret_cast<uint64_t>(data->state.lua_state()));

    eventHandler = std::make_unique<EventHandler>(eventBus);

    setupBindings();

    // Update search paths for the require() function
    auto packagePaths = data->state["package"]["path"].get<std::string>();
    packagePaths = packagePaths + (!packagePaths.empty() ? ";" : "") + (config.assetsPath).string() + "/?.lua";
    data->state["package"]["path"] = packagePaths;
}

Lua::~Lua() {
    instances.erase(data->state.lua_state());
}

void Lua::importModule(const std::string_view& name, const std::string_view& file) {
    const auto path = config.assetsPath / name / file;
    if (Fs::exists(path) && Fs::is_regular_file(path)) {
        data->state.script_file(path.string());
    } else {
        logger.info("No such module script: '{}'", path);
    }
}

void Lua::require(const std::string_view& name) {
    logger.info("Lua require: '{}'", name);

    auto res = data->state["require"](name);
    if (!res.valid()) {
        sol::error err = res;
        EXCEPTION("Lua require: '{}' error: {}", name, err.what());
    }
}

void Lua::require(const std::string_view& name, const std::function<void(sol::table&)>& callback) {
    logger.info("Lua require: '{}'", name);

    auto res = data->state["require"](name);
    if (!res.valid()) {
        sol::error err = res;
        EXCEPTION("Lua require: '{}' error: {}", name, err.what());
    }

    try {
        auto table = res.get<sol::table>();
        callback(table);
    } catch (...) {
        EXCEPTION_NESTED("Lua require: '{}' failed", name);
    }
}

sol::table& Lua::root() {
    return data->engine;
}

sol::state& Lua::getState() {
    return data->state;
}

Scene& Lua::getScene() {
    if (!scene) {
        EXCEPTION("Trying to access scene from a non-sector script");
    }
    return *scene;
}

void Lua::setScene(Scene& value) {
    scene = &value;
}

void Engine::Lua::registerBinder(Lua::Binder& binder) {
    getBinders().push_back(&binder);
}

std::vector<Lua::Binder*>& Lua::getBinders() {
    static std::vector<Binder*> binders;
    return binders;
}

void Lua::setupBindings() {
    auto& m = root();

    EventHandler::bind(*this);
    m["get_scene"] = [this]() -> Scene* { return &getScene(); };
    m["get_event_bus"] = [this]() -> EventHandler* { return eventHandler.get(); };

    for (const auto binder : getBinders()) {
        binder->setup(m);
    }
}
