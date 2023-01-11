#pragma once

#define MOVEABLE(ClassName)                                                                                            \
    ClassName(ClassName&& other) = default;                                                                            \
    ClassName& operator=(ClassName&& other) = default;

#define NON_MOVEABLE(ClassName)                                                                                        \
    ClassName(ClassName&& other) = delete;                                                                             \
    ClassName& operator=(ClassName&& other) = delete;

#define COPYABLE(ClassName)                                                                                            \
    ClassName(const ClassName& other) = default;                                                                       \
    ClassName& operator=(const ClassName& other) = default;

#define NON_COPYABLE(ClassName)                                                                                        \
    ClassName(const ClassName& other) = delete;                                                                        \
    ClassName& operator=(const ClassName& other) = delete;
