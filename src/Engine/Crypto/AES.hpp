#pragma once

#include "../Library.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

typedef struct evp_cipher_st EVP_CIPHER;
typedef struct evp_md_st EVP_MD;
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

namespace Engine {
class ENGINE_API AES {
public:
    static constexpr size_t ivecLength = 16;
    static constexpr size_t keyLength = 32;

    explicit AES(const std::vector<uint8_t>& sharedKey);
    ~AES();

    size_t encrypt(const void* src, void* dst, size_t size);
    size_t decrypt(const void* src, void* dst, size_t size);

private:
    static void evpDeleter(EVP_CIPHER_CTX* p);
    static const EVP_CIPHER* cipher;
    static const EVP_MD* digest;

    std::array<uint8_t, keyLength> key{};
    std::array<uint8_t, ivecLength> ivec{};
};
} // namespace Engine
