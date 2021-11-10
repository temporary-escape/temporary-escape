#pragma once

#include <vector>

namespace Scissio {
template <typename T> class Span {
    const T* ptr;
    std::size_t len;

public:
    Span(const T* ptr, std::size_t len) noexcept : ptr{ptr}, len{len} {
    }

    Span(const std::vector<T>& vec) noexcept : ptr{vec.data()}, len{vec.size()} {
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return len;
    }

    const T* data() const noexcept {
        return ptr;
    }

    const T* begin() noexcept {
        return ptr;
    }

    const T* end() noexcept {
        return ptr + len;
    }
};
}
