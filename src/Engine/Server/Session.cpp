#include "Session.hpp"
#include "Lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

void Session::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Session>("Session");
}
