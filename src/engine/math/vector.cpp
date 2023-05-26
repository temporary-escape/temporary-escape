#include "vector.hpp"
#include "../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

template <typename T> static std::string metaToString(T& self) {
    return fmt::format("{}", self);
}

template <typename T, typename O = T> static T metaAdd(T& self, const O& other) {
    return self + other;
}

template <typename T, typename O = T> static T metaSub(T& self, const O& other) {
    return self + other;
}

template <typename T, typename O = T> static T metaMul(T& self, const O& other) {
    return self * other;
}

template <typename T, typename O = T> static T metaDiv(T& self, const O& other) {
    return self / other;
}

template <typename T> static T metaNeg(T& self) {
    return -self;
}

template <typename T> static bool metaEq(T& self, const T& other) {
    return self == other;
}

template <typename T, typename O> static void metaArithemtic(sol::usertype<T>& cls) {
    cls[sol::meta_function::to_string] = &metaToString<T>;
    cls[sol::meta_function::addition] = sol::overload(&metaAdd<T, T>, &metaAdd<T, O>);
    cls[sol::meta_function::subtraction] = sol::overload(&metaSub<T, T>, &metaSub<T, O>);
    cls[sol::meta_function::division] = sol::overload(&metaDiv<T, T>, &metaDiv<T, O>);
    cls[sol::meta_function::multiplication] = sol::overload(&metaMul<T, T>, &metaMul<T, O>);
    cls[sol::meta_function::unary_minus] = &metaNeg<T>;
    cls[sol::meta_function::equal_to] = &metaEq<T>;
}

void Engine::bindMathVectors(Lua& lua) {
    auto& m = lua.root();

    { // Vector2
        auto cls = m.new_usertype<Vector2>("Vector2", sol::constructors<Vector2(), Vector2(float, float)>{});
        cls["x"] = &Vector2::x;
        cls["y"] = &Vector2::x;
        metaArithemtic<Vector2, float>(cls);
        cls["distance"] = [](Vector2& self, const Vector2& other) { return glm::distance(self, other); };
        cls["rotate"] = [](Vector2& self, const float angle) { return glm::rotate(self, angle); };
    }

    { // Vector3
        auto cls = m.new_usertype<Vector3>("Vector3", sol::constructors<Vector3(), Vector3(float, float, float)>{});
        cls["x"] = &Vector3::x;
        cls["y"] = &Vector3::y;
        cls["z"] = &Vector3::z;
        metaArithemtic<Vector3, float>(cls);
        cls["distance"] = [](Vector3& self, const Vector3& other) { return glm::distance(self, other); };
    }

    { // Vector4
        auto cls =
            m.new_usertype<Vector4>("Vector4", sol::constructors<Vector4(), Vector4(float, float, float, float)>{});
        cls["x"] = &Vector4::x;
        cls["y"] = &Vector4::y;
        cls["z"] = &Vector4::z;
        cls["w"] = &Vector4::w;
        metaArithemtic<Vector4, float>(cls);
        cls["distance"] = [](Vector4& self, const Vector4& other) { return glm::distance(self, other); };
    }
}
