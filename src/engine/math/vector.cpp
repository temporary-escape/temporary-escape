#include "vector.hpp"
#include "../server/lua.hpp"
#include "../server/lua_meta.hpp"

using namespace Engine;

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

    { // Color4
        auto cls = m.new_usertype<Color4>("Color4", sol::constructors<Color4(), Color4(float, float, float, float)>{});
        cls["x"] = &Color4::x;
        cls["y"] = &Color4::y;
        cls["z"] = &Color4::z;
        cls["w"] = &Color4::w;
        metaArithemtic<Color4, float>(cls);
        cls["distance"] = [](Color4& self, const Color4& other) { return glm::distance(self, other); };
    }
}
