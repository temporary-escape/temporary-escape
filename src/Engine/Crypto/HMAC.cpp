#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "HMAC.hpp"
#include "../Utils/Exceptions.hpp"
#include <openssl/evp.h>
#include <openssl/hmac.h>

using namespace Engine;

const EVP_MD* HMAC::digest = EVP_get_digestbyname("sha256");

HMAC::HMAC(const std::vector<uint8_t>& sharedKey) {
    static const std::string_view salt = "HMAC";
    if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(sharedKey.data()),
                          static_cast<int>(sharedKey.size()),
                          reinterpret_cast<const unsigned char*>(salt.data()),
                          static_cast<int>(salt.size()),
                          1000,
                          digest,
                          static_cast<int>(key.size()),
                          key.data()) != 1) {
        EXCEPTION("Failed to get HMAC derived key");
    }
}

HMAC::~HMAC() = default;

size_t HMAC::sign(const void* src, void* dst, const size_t size) const {
    unsigned int length;
    if (!::HMAC(digest,
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
