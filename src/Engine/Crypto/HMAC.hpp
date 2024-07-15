#pragma once

#include "../Library.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Engine {
class ENGINE_API HMAC {
public:
    static constexpr size_t resultSize{32};
    static constexpr size_t keySize{32};

    explicit HMAC(const std::vector<uint8_t>& sharedKey);
    ~HMAC();

    size_t sign(const void* src, void* dst, size_t size) const;

private:
    std::array<uint8_t, keySize> key{};
};
} // namespace Engine
