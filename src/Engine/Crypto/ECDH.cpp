#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "ECDH.hpp"
#include "../Utils/Exceptions.hpp"
#include <openssl/core_names.h>
#include <openssl/dh.h>
#include <openssl/ec.h>
#include <openssl/param_build.h>
#include <openssl/pem.h>

using namespace Engine;

static bool isCurveSupported(const std::string_view& name) {
    const auto count = EC_get_builtin_curves(nullptr, 0);
    if (count <= 0) {
        EXCEPTION("EC_get_builtin_curves failed to get curves");
    }

    auto buffer = std::make_unique<EC_builtin_curve[]>(count);
    if (EC_get_builtin_curves(buffer.get(), count) != count) {
        EXCEPTION("EC_get_builtin_curves failed to get curves");
    }

    for (size_t i = 0; i < count; i++) {
        const auto* n = OBJ_nid2sn(buffer[i].nid);
        if (std::strncmp(n, name.data(), name.size()) == 0) {
            return true;
        }
    }

    return false;
}

using OsslParamBldPtr = std::unique_ptr<OSSL_PARAM_BLD, decltype(&OSSL_PARAM_BLD_free)>;
using OsslParamPtr = std::unique_ptr<OSSL_PARAM, decltype(&OSSL_PARAM_free)>;
using EvpPkeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using BioPtr = std::unique_ptr<BIO, decltype(&BIO_free)>;

bool ECDH::isPublicKey(const std::string_view& other) {
    static const std::string_view start = "-----BEGIN PUBLIC KEY-----";
    static const std::string_view end = "-----END PUBLIC KEY-----";
    if (other.size() < start.size() + end.size() + 1) {
        return false;
    }

    if (std::strncmp(other.data(), start.data(), start.size()) != 0) {
        return false;
    }

    if (std::strncmp(other.data() + other.size() - end.size() - 1, end.data(), end.size()) != 0) {
        return false;
    }

    return true;
}

ECDH::ECDH(const std::string_view& curve) : evp{nullptr, &evpDeleter} {
    if (!isCurveSupported(curve)) {
        EXCEPTION("The ECDH curve: '{}' is not supported", curve);
    }

    OsslParamBldPtr paramBuild{OSSL_PARAM_BLD_new(), &OSSL_PARAM_BLD_free};
    if (!paramBuild) {
        EXCEPTION("OSSL_PARAM_BLD_new failed to generate param builder");
    }

    if (!OSSL_PARAM_BLD_push_utf8_string(paramBuild.get(), OSSL_PKEY_PARAM_GROUP_NAME, curve.data(), curve.size())) {
        EXCEPTION("OSSL_PARAM_BLD_push failed to set curve name");
    }

    OsslParamPtr params{OSSL_PARAM_BLD_to_param(paramBuild.get()), &OSSL_PARAM_free};
    if (!params) {
        EXCEPTION("OSSL_PARAM_BLD_to_param failed to convert to params");
    }

    EvpPkeyCtxPtr ctx{EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), &EVP_PKEY_CTX_free};
    if (!ctx) {
        EXCEPTION("EVP_PKEY_CTX_new_from_name failed to generate context");
    }

    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
        EXCEPTION("EVP_PKEY_keygen_init failed to initialize context");
    }

    if (!EVP_PKEY_CTX_set_params(ctx.get(), params.get())) {
        EXCEPTION("EVP_PKEY_CTX_set_params failed to set params");
    }

    EVP_PKEY* keyPair = nullptr;
    if (EVP_PKEY_generate(ctx.get(), &keyPair) <= 0) {
        EXCEPTION("EVP_PKEY_generate failed to generate keypair");
    }

    evp.reset(keyPair);
}

ECDH::~ECDH() {
}

std::string ECDH::getPublicKey() const {
    BioPtr bio{BIO_new(BIO_s_mem()), &BIO_free};

    if (!PEM_write_bio_PUBKEY(bio.get(), evp.get())) {
        EXCEPTION("Failed to read public key");
    }

    const auto len = BIO_pending(bio.get());
    if (!len) {
        EXCEPTION("Failed to read public key length");
    }

    std::string result;
    result.resize(len);
    if (!BIO_read(bio.get(), &result[0], result.size())) {
        EXCEPTION("Failed to read public key data");
    }

    return result;
}

std::vector<uint8_t> ECDH::deriveSharedSecret(const std::string_view& other) {
    BioPtr bio{BIO_new(BIO_s_mem()), &BIO_free};

    if (!BIO_write(bio.get(), other.data(), other.size())) {
        EXCEPTION("BIO_write failed to read public key data");
    }

    auto ptr = PEM_read_bio_EC_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    if (!ptr) {
        EXCEPTION("PEM_read_bio_EC_PUBKEY failed to parse public key");
    }

    EvpKeyPtr pubKey{EVP_PKEY_new(), &evpDeleter};
    if (!EVP_PKEY_assign_EC_KEY(pubKey.get(), ptr)) {
        EC_KEY_free(ptr);
        EXCEPTION("EVP_PKEY_assign_EC_KEY failed");
    }

    EvpPkeyCtxPtr ctx{EVP_PKEY_CTX_new(evp.get(), nullptr), &EVP_PKEY_CTX_free};
    if (!ctx) {
        EXCEPTION("EVP_PKEY_CTX_new failed to create derivation context");
    }

    if (EVP_PKEY_derive_init(ctx.get()) <= 0) {
        EXCEPTION("EVP_PKEY_derive_init failed to initialize derivation context");
    }

    if (EVP_PKEY_derive_set_peer(ctx.get(), pubKey.get()) <= 0) {
        EXCEPTION("EVP_PKEY_derive_set_peer failed to set peer public key");
    }

    size_t length{0};
    if (EVP_PKEY_derive(ctx.get(), nullptr, &length) <= 0 || length == 0) {
        EXCEPTION("EVP_PKEY_derive failed to get shared secret length");
    }

    std::vector<uint8_t> result;
    result.resize(length);

    if (EVP_PKEY_derive(ctx.get(), &result[0], &length) <= 0 || length == 0) {
        EXCEPTION("EVP_PKEY_derive failed to get shared secret data");
    }

    return result;
}

void ECDH::evpDeleter(EVP_PKEY* p) {
    EVP_PKEY_free(p);
}

#pragma clang diagnostic pop
