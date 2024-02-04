#pragma once

#include <type_traits>

#define EXPAND(x) x
#define PP_NARG(...) EXPAND(PP_NARG_(__VA_ARGS__, PP_RSEQ_N()))
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1,                                                                                                   \
                 _2,                                                                                                   \
                 _3,                                                                                                   \
                 _4,                                                                                                   \
                 _5,                                                                                                   \
                 _6,                                                                                                   \
                 _7,                                                                                                   \
                 _8,                                                                                                   \
                 _9,                                                                                                   \
                 _10,                                                                                                  \
                 _11,                                                                                                  \
                 _12,                                                                                                  \
                 _13,                                                                                                  \
                 _14,                                                                                                  \
                 _15,                                                                                                  \
                 _16,                                                                                                  \
                 _17,                                                                                                  \
                 _18,                                                                                                  \
                 _19,                                                                                                  \
                 _20,                                                                                                  \
                 _21,                                                                                                  \
                 _22,                                                                                                  \
                 _23,                                                                                                  \
                 _24,                                                                                                  \
                 _25,                                                                                                  \
                 _26,                                                                                                  \
                 _27,                                                                                                  \
                 _28,                                                                                                  \
                 _29,                                                                                                  \
                 _30,                                                                                                  \
                 _31,                                                                                                  \
                 _32,                                                                                                  \
                 _33,                                                                                                  \
                 _34,                                                                                                  \
                 _35,                                                                                                  \
                 _36,                                                                                                  \
                 _37,                                                                                                  \
                 _38,                                                                                                  \
                 _39,                                                                                                  \
                 _40,                                                                                                  \
                 _41,                                                                                                  \
                 _42,                                                                                                  \
                 _43,                                                                                                  \
                 _44,                                                                                                  \
                 _45,                                                                                                  \
                 _46,                                                                                                  \
                 _47,                                                                                                  \
                 _48,                                                                                                  \
                 _49,                                                                                                  \
                 _50,                                                                                                  \
                 _51,                                                                                                  \
                 _52,                                                                                                  \
                 _53,                                                                                                  \
                 _54,                                                                                                  \
                 _55,                                                                                                  \
                 _56,                                                                                                  \
                 _57,                                                                                                  \
                 _58,                                                                                                  \
                 _59,                                                                                                  \
                 _60,                                                                                                  \
                 _61,                                                                                                  \
                 _62,                                                                                                  \
                 _63,                                                                                                  \
                 N,                                                                                                    \
                 ...)                                                                                                  \
    N
#define PP_RSEQ_N()                                                                                                    \
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,    \
        35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8,  \
        7, 6, 5, 4, 3, 2, 1, 0

#define XSTR(x) #x

#define FOR_EACH_EXPAND(x) x
#define FOR_EACH_1(what, x, ...) what(x)
#define FOR_EACH_2(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_1(what, __VA_ARGS__))
#define FOR_EACH_3(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_2(what, __VA_ARGS__))
#define FOR_EACH_4(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_3(what, __VA_ARGS__))
#define FOR_EACH_5(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_4(what, __VA_ARGS__))
#define FOR_EACH_6(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_5(what, __VA_ARGS__))
#define FOR_EACH_7(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_6(what, __VA_ARGS__))
#define FOR_EACH_8(what, x, ...) what(x) FOR_EACH_EXPAND(FOR_EACH_7(what, __VA_ARGS__))

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_EXPAND(FOR_EACH_ARG_N(__VA_ARGS__))
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define FOR_EACH_CONCATENATE(x, y) x##y
#define FOR_EACH_(N, what, ...) FOR_EACH_EXPAND(FOR_EACH_CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__))
#define FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#define FOR_EACH_PASS_1(what, p, x, ...) what(p, x)
#define FOR_EACH_PASS_2(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_1(what, p, __VA_ARGS__))
#define FOR_EACH_PASS_3(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_2(what, p, __VA_ARGS__))
#define FOR_EACH_PASS_4(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_3(what, p, __VA_ARGS__))
#define FOR_EACH_PASS_5(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_4(what, p, __VA_ARGS__))
#define FOR_EACH_PASS_6(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_5(what, p, __VA_ARGS__))
#define FOR_EACH_PASS_7(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_6(what, p, __VA_ARGS__))
#define FOR_EACH_PASS_8(what, p, x, ...) what(p, x) FOR_EACH_EXPAND(FOR_EACH_PASS_7(what, p, __VA_ARGS__))

#define FOR_EACH_PASS_(N, what, p, ...) FOR_EACH_EXPAND(FOR_EACH_CONCATENATE(FOR_EACH_PASS_, N)(what, p, __VA_ARGS__))
#define FOR_EACH_PASS(what, p, ...) FOR_EACH_PASS_(FOR_EACH_NARG(__VA_ARGS__), what, p, __VA_ARGS__)

#define ENUM_TO_STRING_ENTRY(p, e) {p::e, #e},
#define ENUM_TO_STRING(E, ...)                                                                                         \
    inline const std::string& toString(const E value) {                                                                \
        static const std::unordered_map<E, std::string> map = {FOR_EACH_PASS(ENUM_TO_STRING_ENTRY, E, __VA_ARGS__)};   \
        return map.at(value);                                                                                          \
    }

#define DEFINE_ENUM_FLAG_OPERATORS(Type)                                                                               \
    inline Type operator~(Type a) {                                                                                    \
        return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(a));                                       \
    }                                                                                                                  \
    inline Type operator|(Type a, Type b) {                                                                            \
        return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(a) |                                        \
                                 static_cast<std::underlying_type_t<Type>>(b));                                        \
    }                                                                                                                  \
    inline Type operator&(Type a, Type b) {                                                                            \
        return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(a) &                                        \
                                 static_cast<std::underlying_type_t<Type>>(b));                                        \
    }                                                                                                                  \
    inline Type operator^(Type a, Type b) {                                                                            \
        return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(a) ^                                        \
                                 static_cast<std::underlying_type_t<Type>>(b));                                        \
    }                                                                                                                  \
    inline Type& operator|=(Type& a, Type b) {                                                                         \
        return reinterpret_cast<Type&>(reinterpret_cast<std::underlying_type_t<Type>&>(a) |=                           \
                                       static_cast<std::underlying_type_t<Type>>(b));                                  \
    }                                                                                                                  \
    inline Type& operator&=(Type& a, Type b) {                                                                         \
        return reinterpret_cast<Type&>(reinterpret_cast<std::underlying_type_t<Type>&>(a) &=                           \
                                       static_cast<std::underlying_type_t<Type>>(b));                                  \
    }                                                                                                                  \
    inline Type& operator^=(Type& a, Type b) {                                                                         \
        return reinterpret_cast<Type&>(reinterpret_cast<std::underlying_type_t<Type>&>(a) ^=                           \
                                       static_cast<std::underlying_type_t<Type>>(b));                                  \
    }\
