#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "HMAC.hpp"
#include "../Utils/Exceptions.hpp"
#include "HKDF.hpp"
#include <openssl/evp.h>
#include <openssl/hmac.h>

using namespace Engine;

const EVP_MD* HMAC::digest = EVP_get_digestbyname("sha256");

HMAC::HMAC(const std::vector<uint8_t>& sharedKey) {
    static const std::string_view salt = "hmac";
    auto result = hkdfKeyDerivation(sharedKey, salt);

    std::memcpy(key.data(), result.data(), 32);
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
