#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "HKDF.hpp"
#include "../Utils/Exceptions.hpp"

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>

using namespace Engine;

using KdfCtxPtr = std::unique_ptr<EVP_KDF_CTX, decltype(&EVP_KDF_CTX_free)>;

HkdfResult Engine::hkdfKeyDerivation(const std::vector<uint8_t>& sharedSecret, const std::string_view& salt) {
    static const std::string_view algo = "HKDF";
    static const std::string_view digest = "sha256";
    static const std::string_view info = "label";

    // Find and allocate a context for the HKDF algorithm
    auto kdf = EVP_KDF_fetch(nullptr, algo.data(), nullptr);
    if (!kdf) {
        EXCEPTION("EVP_KDF_fetch failed to fetch hkdf algorithm");
    }

    auto kctxPtr = EVP_KDF_CTX_new(kdf);

    // The kctx keeps a reference so this is safe
    EVP_KDF_free(kdf);

    if (!kctxPtr) {
        EXCEPTION("EVP_KDF_CTX_new failed to create context");
    }

    KdfCtxPtr kctx{kctxPtr, &EVP_KDF_CTX_free};

    // Cast to types that OSSL_PARAM_construct can accept
    auto digestData = const_cast<char*>(digest.data());
    auto infoData = const_cast<char*>(info.data());
    auto saltData = const_cast<char*>(reinterpret_cast<const char*>(salt.data()));
    auto secretData = const_cast<uint8_t*>(sharedSecret.data());

    // Build up the parameters for the derivation
    OSSL_PARAM params[5];
    auto* p = params;
    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST, digestData, digest.size());
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, saltData, salt.size());
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_KEY, secretData, sharedSecret.size());
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_INFO, infoData, info.size());
    *p = OSSL_PARAM_construct_end();

    if (EVP_KDF_CTX_set_params(kctx.get(), params) <= 0) {
        EXCEPTION("EVP_KDF_CTX_set_params failed to set params");
    }

    HkdfResult result;

    // Do the derivation
    if (EVP_KDF_derive(kctx.get(), result.data(), result.size(), nullptr) <= 0) {
        EXCEPTION("EVP_KDF_derive failed");
    }

    return result;
}
