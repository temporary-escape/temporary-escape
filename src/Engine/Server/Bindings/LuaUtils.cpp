#include "../../Utils/NameGenerator.hpp"
#include "../../Utils/Random.hpp"
#include "Bindings.hpp"

using namespace Engine;

static std::shared_ptr<NameGenerator> createNameGenerator(sol::as_table_t<std::vector<std::string>> arg) {
    return std::make_shared<NameGenerator>(arg.value());
}

static void bindNameGenerator(sol::table& m) {
    auto cls = m.new_usertype<NameGenerator>("NameGenerator", sol::factories(&createNameGenerator));
    cls["get"] = &NameGenerator::operator();
}

LUA_BINDINGS(bindNameGenerator);

static void bindLogger(sol::table& m) {
    auto cls = m.new_usertype<Logger>("Logger");
    cls["info"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::info);
    cls["debug"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::debug);
    cls["warn"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::warn);
    cls["error"] = static_cast<void (Logger::*)(const std::string&)>(&Logger::error);
}

LUA_BINDINGS(bindLogger);

static void bindUtilsFunctions(sol::table& m) {
    m["uuid"] = &uuid;
}

LUA_BINDINGS(bindUtilsFunctions);
