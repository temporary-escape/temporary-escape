#pragma once

#include "../Library.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

namespace Engine {
class ENGINE_API AES {
public:
    AES(const std::vector<uint8_t>& sharedKey);
    ~AES();

    static constexpr size_t getEncryptSize(size_t size) {
        return size + ivecLength;
        /*if (size % 16 == 0) {
            return size + ivecLength;
        } else {
            return size + 16 - (size % 16) + ivecLength;
        }*/
    }

    static constexpr size_t getDecryptSize(size_t size) {
        if (size < ivecLength) {
            return 0;
        }
        return size - ivecLength;
    }

    size_t encrypt(const void* src, void* dst, size_t size);
    size_t decrypt(const void* src, void* dst, size_t size);

private:
    static constexpr size_t ivecLength = 16;

    static void evpDeleter(EVP_CIPHER_CTX* p);

    std::unique_ptr<EVP_CIPHER_CTX, decltype(&evpDeleter)> ctx;
    std::array<uint8_t, 64> key{};
    std::array<uint8_t, ivecLength> ivec{};
};
} // namespace Engine
