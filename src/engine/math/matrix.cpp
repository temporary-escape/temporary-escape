#include "matrix.hpp"
#include "../server/lua.hpp"
#include "../server/lua_meta.hpp"

using namespace Engine;

void Engine::bindMathMatrices(Lua& lua) {
    auto& m = lua.root();

    { // Matrix4
        auto cls = m.new_usertype<Matrix4>("Matrix4", sol::constructors<Matrix4()>{});
    }
}
