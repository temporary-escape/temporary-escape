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

    /**
     * @module engine
     */

    {
        /**
         * @class Vector2
         * A simple 2D vector class of float type
         */
        /**
         * @function Vector2.new
         * Default constructor that initializes the vector to [0, 0]
         */
        /**
         * @function Vector2.new
         * Parametrized constructor that initializes the vector to [x, y]
         * @param x number The x component
         * @param y number The y component
         */
        auto cls = m.new_usertype<Vector2>("Vector2", sol::constructors<Vector2(), Vector2(float, float)>{});
        /**
         * @field Vector2.x
         * @type number
         */
        cls["x"] = &Vector2::x;
        /**
         * @field Vector2.y
         * @type number
         */
        cls["y"] = &Vector2::x;
        metaArithemtic<Vector2, float>(cls);
        /**
         * @function Vector2:distance
         * Calculates a distance to some other vector
         * @param other Vector2 The other vector to get distance to
         * @return number The distance
         */
        cls["distance"] = [](Vector2& self, const Vector2& other) { return glm::distance(self, other); };
        /**
         * @function Vector2:rotate
         * Returns a new vector that is rotated by some angle
         * @param angle number in radians
         * @return Vector2 A new rotated vector
         * @code
         * local PI = 3.141592653589
         * local a = engine.Vector2.new(1.0, 0.0)
         * local b = a:rotate(PI)
         * -- Prints: Rotated vector [1, 0] by 180 degrees is: [-1, -8.742278e-08]
         * print(string.format("Rotated vector %s by 180 degrees is: %s", a, b))
         * @endcode
         */
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
