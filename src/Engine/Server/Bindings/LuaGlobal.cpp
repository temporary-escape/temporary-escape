#include "../../Assets/AssetsManager.hpp"
#include "../../Database/Database.hpp"
#include "../../Utils/NameGenerator.hpp"
#include "../../Utils/Random.hpp"
#include "../Server.hpp"
#include "Bindings.hpp"

using namespace Engine;

static Server* getServerHelper() {
    return Server::instance;
}

static Generator* getGeneratorHelper() {
    return &Server::instance->getGenerator();
}

static AssetsManager* getAssetManagerHelper() {
    return &Server::instance->getAssetManager();
}

static Database* getDatabase() {
    return &Server::instance->getDatabase();
}

static void bindGlobalFunctions(sol::table& m) {
    m["create_logger"] = &createLogger;
    m["get_server"] = &getServerHelper;
    m["get_generator"] = &getGeneratorHelper;
    m["get_database"] = &getDatabase;
    m["get_assets_manager"] = &getAssetManagerHelper;
}

LUA_BINDINGS(bindGlobalFunctions);
