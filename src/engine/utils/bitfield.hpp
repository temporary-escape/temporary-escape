#pragma once

#include <cstdint>

namespace Engine {
template <typename T, uint64_t offset, uint64_t length> class Bitfield {
public:
    static constexpr uint64_t bits = (1 << length) - 1;
    static constexpr uint64_t mask = bits << offset;

    inline Bitfield& operator=(const T other) {
        auto& value = self();
        value = (value & ~mask) | ((static_cast<uint64_t>(other) & bits) << offset);
        return *this;
    }

    inline Bitfield& operator=(const Bitfield<T, offset, length>& other) {
        *this = T(other);
        return *this;
    }

    inline operator T() const {
        auto& value = self();
        return static_cast<T>((value & mask) >> offset);
    }

    [[nodiscard]] inline uint64_t value() const {
        auto& value = self();
        return static_cast<uint64_t>((value & mask) >> offset);
    }

private:
    uint64_t& self() {
        return *reinterpret_cast<uint64_t*>(this);
    }

    const uint64_t& self() const {
        return *reinterpret_cast<const uint64_t*>(this);
    }
};
} // namespace Engine
