#pragma once

#include "../../Utils/Format.hpp"
#include "../Lua.hpp"
#include <sol/sol.hpp>

namespace Engine {
template <typename T> static std::string metaToString(T& self) {
    return fmt::format("{}", self);
}

template <typename T, typename O = T> static T metaAdd(T& self, const O& other) {
    return self + other;
}

template <typename T, typename O = T> static T metaSub(T& self, const O& other) {
    return self - other;
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
} // namespace Engine

#define LUA_CALL_FN(fn, ...)                                                                                           \
    sol::protected_function_result result = fn(__VA_ARGS__);                                                           \
    if (!result.valid()) {                                                                                             \
        sol::error err = result;                                                                                       \
        EXCEPTION("{}", err.what());                                                                                   \
    }
