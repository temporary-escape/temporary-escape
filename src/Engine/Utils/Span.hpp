#pragma once

#if __cplusplus >= 202002L
#include <span>

namespace Engine {
template <typename T> using Span = std::span<T>;
}
#else
#include <vector>

namespace Engine {
template <typename T> class Span {
    const T* ptr = nullptr;
    std::size_t len = 0;

public:
    Span() = default;

    Span(const T* ptr, std::size_t len) noexcept : ptr{ptr}, len{len} {
    }

    Span(const std::vector<T>& vec) noexcept : ptr{vec.data()}, len{vec.size()} {
    }

    template <size_t N> Span(const std::array<T, N>& arr) noexcept : ptr{arr.data()}, len{arr.size()} {
    }

    template <size_t N> Span(const T (&arr)[N]) noexcept : ptr{arr}, len{N} {
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return len;
    }

    const T* data() const noexcept {
        return ptr;
    }

    const T* begin() const noexcept {
        return ptr;
    }

    const T* end() const noexcept {
        return ptr + len;
    }

    bool empty() const {
        return len == 0;
    }

    T& operator[](const size_t i) {
        return ptr[i];
    }

    const T& operator[](const size_t i) const {
        return ptr[i];
    }
};
} // namespace Engine
#endif
