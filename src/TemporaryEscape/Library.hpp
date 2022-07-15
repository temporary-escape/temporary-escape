#pragma once

#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#define ENGINE_API
#endif

#define NON_COPYABLE(T)                                                                                                \
    T(const T& other) = delete;                                                                                        \
    T& operator=(const T& other) = delete;\
