#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "HMAC.hpp"
#include "../Utils/Exceptions.hpp"
#include <openssl/evp.h>
#include <openssl/hmac.h>

Engine::HMAC::HMAC(const std::vector<uint8_t>& sharedKey) {
    static const std::string_view salt = "HMAC";
    if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(sharedKey.data()),
                          static_cast<int>(sharedKey.size()),
                          reinterpret_cast<const unsigned char*>(salt.data()),
                          static_cast<int>(salt.size()),
                          4096,
                          EVP_sha256(),
                          static_cast<int>(key.size()),
                          key.data()) != 1) {
        EXCEPTION("Failed to get HMAC derived key");
    }
}

Engine::HMAC::~HMAC() = default;

size_t Engine::HMAC::sign(const void* src, void* dst, const size_t size) const {
    unsigned int length;
    if (!::HMAC(EVP_sha256(),
                key.data(),
                static_cast<int>(key.size()),
                static_cast<const unsigned char*>(src),
                size,
                static_cast<unsigned char*>(dst),
                &length)) {
        return 0;
    }
    return length;
}
