#include "quaternion.hpp"
#include "../server/lua.hpp"
#include "../server/lua_meta.hpp"
#include <cmath>

using namespace Engine;

static const float PI = static_cast<float>(std::atan(1) * 4);

Quaternion Engine::randomQuaternion(std::mt19937_64& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const auto u = dist(rng);
    const auto v = dist(rng);
    const auto w = dist(rng);

    Quaternion q;
    auto* data = const_cast<float*>(&q.x);

    data[0] = std::sqrt(1.0f - u) * std::sin(2.0f * PI * v);
    data[1] = std::sqrt(1.0f - u) * std::cos(2.0f * PI * v);
    data[2] = std::sqrt(u) * std::sin(2.0f * PI * w);
    data[3] = std::sqrt(u) * std::cos(2.0f * PI * w);

    return q;
}

void Engine::bindMathQuaternion(Lua& lua) {
    auto& m = lua.root();

    { // Quaternion
        auto cls = m.new_usertype<Quaternion>(
            "Quaternion", sol::constructors<Quaternion(), Quaternion(float, float, float, float)>{});
        cls["x"] = &Quaternion::x;
        cls["y"] = &Quaternion::y;
        cls["z"] = &Quaternion::z;
        cls["w"] = &Quaternion::w;
        cls[sol::meta_function::to_string] = &metaToString<Quaternion>;
        cls[sol::meta_function::multiplication] = &metaMul<Quaternion, Quaternion>;
        cls[sol::meta_function::equal_to] = &metaEq<Quaternion>;
    }
}
