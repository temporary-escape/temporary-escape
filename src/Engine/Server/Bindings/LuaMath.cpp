#include "../../Math/Matrix.hpp"
#include "../../Math/Quaternion.hpp"
#include "../../Math/Random.hpp"
#include "../../Math/Vector.hpp"
#include "Bindings.hpp"

using namespace Engine;

static void bindMathVectors(sol::table& m) {
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
        cls["x"] = LUA_PROP(&Vector2::x);
        /**
         * @field Vector2.y
         * @type number
         */
        cls["y"] = LUA_PROP(&Vector2::x);
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
        cls["x"] = LUA_PROP(&Vector3::x);
        cls["y"] = LUA_PROP(&Vector3::y);
        cls["z"] = LUA_PROP(&Vector3::z);
        metaArithemtic<Vector3, float>(cls);
        cls["distance"] = [](Vector3& self, const Vector3& other) { return glm::distance(self, other); };
    }

    { // Vector4
        auto cls =
            m.new_usertype<Vector4>("Vector4", sol::constructors<Vector4(), Vector4(float, float, float, float)>{});
        cls["x"] = LUA_PROP(&Vector4::x);
        cls["y"] = LUA_PROP(&Vector4::y);
        cls["z"] = LUA_PROP(&Vector4::z);
        cls["w"] = LUA_PROP(&Vector4::w);
        metaArithemtic<Vector4, float>(cls);
        cls["distance"] = [](Vector4& self, const Vector4& other) { return glm::distance(self, other); };
    }

    { // Color4
        auto cls = m.new_usertype<Color4>("Color4", sol::constructors<Color4(), Color4(float, float, float, float)>{});
        cls["x"] = LUA_PROP(&Color4::x);
        cls["y"] = LUA_PROP(&Color4::y);
        cls["z"] = LUA_PROP(&Color4::z);
        cls["w"] = LUA_PROP(&Color4::w);
        metaArithemtic<Color4, float>(cls);
        cls["distance"] = [](Color4& self, const Color4& other) { return glm::distance(self, other); };
    }
}

LUA_BINDINGS(bindMathVectors);

static void bindMathQuaternion(sol::table& m) {
    { // Quaternion
        auto cls = m.new_usertype<Quaternion>(
            "Quaternion", sol::constructors<Quaternion(), Quaternion(float, float, float, float)>{});
        cls["x"] = LUA_PROP(&Quaternion::x);
        cls["y"] = LUA_PROP(&Quaternion::y);
        cls["z"] = LUA_PROP(&Quaternion::z);
        cls["w"] = LUA_PROP(&Quaternion::w);
        cls[sol::meta_function::to_string] = &metaToString<Quaternion>;
        cls[sol::meta_function::multiplication] = &metaMul<Quaternion, Quaternion>;
        cls[sol::meta_function::equal_to] = &metaEq<Quaternion>;
    }
}

LUA_BINDINGS(bindMathQuaternion);

static void bindMathMatrices(sol::table& m) {
    { // Matrix4
        auto cls = m.new_usertype<Matrix4>("Matrix4", sol::constructors<Matrix4()>{});
    }
}

LUA_BINDINGS(bindMathMatrices);

static void bindMathFunctions(sol::table& m) {
    m["radians"] = glm::radians<float>;
    m["degrees"] = glm::degrees<float>;
}

LUA_BINDINGS(bindMathFunctions);

static void bindMathRandom(sol::table& m) {
    m["random_circle_positions"] = &randomCirclePositions;

    { // std::mt19937_64
        auto cls = m.new_usertype<std::mt19937_64>("MT19937", sol::constructors<std::mt19937_64(uint64_t)>{});
        cls["rand_int"] = [](std::mt19937_64& self, const int64_t min, const int64_t max) {
            return randomInt<int64_t>(self, min, max);
        };
        cls["rand_real"] = [](std::mt19937_64& self, const float min, const float max) {
            return randomReal<float>(self, min, max);
        };
        cls["rand_seed"] = [](std::mt19937_64& self) { return randomInt<uint64_t>(self, 0, 0x1FFFFFFFFFFFFF); };
        cls["rand_quaternion"] = [](std::mt19937_64& self) { return randomQuaternion(self); };
    }
}

LUA_BINDINGS(bindMathRandom);
