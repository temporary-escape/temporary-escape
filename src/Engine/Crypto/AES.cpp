#include "AES.hpp"
#include "../Utils/Exceptions.hpp"
#include <cstring>
#include <openssl/evp.h>

using namespace Engine;

AES::AES(const std::vector<uint8_t>& sharedKey) {
    const unsigned char* salt = nullptr;

    static const auto cipher = EVP_get_cipherbyname("aes-128-ctr");
    if (!cipher) {
        EXCEPTION("Failed to get cipher by name");
    }

    static const auto digest = EVP_get_digestbyname("sha256");
    if (!cipher) {
        EXCEPTION("Failed to get digest by name");
    }

    if (!EVP_BytesToKey(
            cipher, digest, salt, sharedKey.data(), static_cast<int>(sharedKey.size()), 1, &key[0], &ivec[0])) {
        EXCEPTION("Failed to derive key and iv from the shared secret");
    }
}

AES::~AES() = default;

size_t AES::encrypt(const void* src, void* dst, const size_t size) {
    std::memcpy(dst, ivec.data(), ivecLength);

    std::unique_ptr<EVP_CIPHER_CTX, decltype(&evpDeleter)> ctx{EVP_CIPHER_CTX_new(), &evpDeleter};

    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_ctr(), nullptr, key.data(), ivec.data()) != 1) {
        return 0;
    }

    int result;
    if (EVP_EncryptUpdate(ctx.get(),
                          reinterpret_cast<unsigned char*>(dst) + ivecLength,
                          &result,
                          reinterpret_cast<const unsigned char*>(src),
                          static_cast<int>(size)) != 1 ||
        result < 0) {
        return 0;
    }

    int final;
    if (EVP_EncryptFinal_ex(ctx.get(), reinterpret_cast<unsigned char*>(dst) + ivecLength + result, &final) != 1) {
        return 0;
    }

    (void)EVP_CIPHER_CTX_get_updated_iv(ctx.get(), &ivec[0], ivec.size());

    return static_cast<size_t>(result) + static_cast<size_t>(final) + ivecLength;
}

size_t AES::decrypt(const void* src, void* dst, const size_t size) {
    if (size <= ivecLength) {
        return 0;
    }

    std::unique_ptr<EVP_CIPHER_CTX, decltype(&evpDeleter)> ctx{EVP_CIPHER_CTX_new(), &evpDeleter};

    const auto start = reinterpret_cast<const unsigned char*>(src);

    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_ctr(), nullptr, key.data(), start) != 1) {
        return 0;
    }

    int result;
    if (EVP_DecryptUpdate(ctx.get(),
                          reinterpret_cast<unsigned char*>(dst),
                          &result,
                          start + ivecLength,
                          static_cast<int>(size - ivecLength)) != 1 ||
        result < 0) {
        return 0;
    }

    int final;
    if (EVP_DecryptFinal_ex(ctx.get(), reinterpret_cast<unsigned char*>(dst) + result, &final) != 1) {
        return 0;
    }

    return static_cast<size_t>(result) + static_cast<size_t>(final);
}

void AES::evpDeleter(EVP_CIPHER_CTX* p) {
    EVP_CIPHER_CTX_free(p);
}
