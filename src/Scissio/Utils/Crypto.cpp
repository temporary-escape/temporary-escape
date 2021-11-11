#include "Crypto.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

using namespace Scissio;

struct Crypto::Ecdhe::Data {
    std::shared_ptr<EVP_PKEY_CTX> ctx;
    std::shared_ptr<EVP_PKEY_CTX> keyCtx;
    std::shared_ptr<EVP_PKEY> key;
};

Crypto::Ecdhe::Ecdhe() : data(std::make_unique<Data>()) {
    data->ctx = std::shared_ptr<EVP_PKEY_CTX>(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), [](EVP_PKEY_CTX* p) { EVP_PKEY_CTX_free(p); });

    if (!data->ctx) {
        EXCEPTION("Failed to create ECDHE context");
    }

    if (!EVP_PKEY_paramgen_init(data->ctx.get())) {
        EXCEPTION("Failed to set ECDHE paramgen");
    }

    if (!EVP_PKEY_CTX_set_ec_paramgen_curve_nid(data->ctx.get(), NID_X9_62_prime256v1)) {
        EXCEPTION("Failed to set ECDHE curve");
    }

    EVP_PKEY* paramsPtr = nullptr;
    if (!EVP_PKEY_paramgen(data->ctx.get(), &paramsPtr)) {
        EXCEPTION("Failed to generate ECDHE params");
    }

    auto params = std::shared_ptr<EVP_PKEY>(paramsPtr, [](EVP_PKEY* p) { EVP_PKEY_free(p); });

    data->keyCtx = std::shared_ptr<EVP_PKEY_CTX>(EVP_PKEY_CTX_new(params.get(), nullptr), [](EVP_PKEY_CTX* p) { EVP_PKEY_CTX_free(p); });
    if (!data->keyCtx) {
        EXCEPTION("Failed to generate ECDHE key context");
    }

    if (!EVP_PKEY_keygen_init(data->keyCtx.get())) {
        EXCEPTION("Failed to initialize ECDHE key context");
    }

    EVP_PKEY* keyPtr = nullptr;
    if (!EVP_PKEY_keygen(data->keyCtx.get(), &keyPtr)) {
        EXCEPTION("Failed to retrieve ECDHE key");
    }

    data->key = std::shared_ptr<EVP_PKEY>(keyPtr, [](EVP_PKEY* p){ EVP_PKEY_free(p); });
}

Crypto::Ecdhe::~Ecdhe() = default;

Crypto::SharedSecret Crypto::Ecdhe::computeSharedSecret(const Crypto::Key& other) {
    auto b = std::shared_ptr<BIO>(BIO_new_mem_buf(reinterpret_cast<const void*>(other.data()), static_cast<int>(other.size())), [](BIO* b) { BIO_free(b); });

    EVP_PKEY* peerKeyPtr = PEM_read_bio_PUBKEY(b.get(), nullptr, nullptr, nullptr);
    if (!peerKeyPtr) {
        EXCEPTION("Failed to read peer public key");
    }

    auto peerKey = std::shared_ptr<EVP_PKEY>(peerKeyPtr, [](EVP_PKEY* p){ EVP_PKEY_free(p); });

    auto ctx = std::shared_ptr<EVP_PKEY_CTX>(EVP_PKEY_CTX_new(data->key.get(), nullptr), [](EVP_PKEY_CTX* p){ EVP_PKEY_CTX_free(p); });

    if (!ctx) {
        EXCEPTION("Failed to create derive context");
    }

    if (!EVP_PKEY_derive_init(ctx.get())) {
        EXCEPTION("Failed to init derive context");
    }

    if (!EVP_PKEY_derive_set_peer(ctx.get(), peerKey.get())) {
        EXCEPTION("Failed to set peer key");
    }

    size_t secretLen;
    if (!EVP_PKEY_derive(ctx.get(), nullptr, &secretLen)) {
        EXCEPTION("Failed to derive shared secret");
    }

    SharedSecret secret;
    secret.resize(secretLen);

    if (!EVP_PKEY_derive(ctx.get(), secret.data(), &secretLen)) {
        EXCEPTION("Failed to derive shared secret");
    }

    return secret;
}

Crypto::Key Crypto::Ecdhe::getPublicKey() {
    auto b = std::shared_ptr<BIO>(BIO_new(BIO_s_mem()), [](BIO* b) { BIO_free(b); });

    if (!PEM_write_bio_PUBKEY(b.get(), data->key.get())) {
        EXCEPTION("Failed to read public key");
    }

    Key result;
    const auto len = BIO_pending(b.get());
    result.resize(len);
    if (!BIO_read(b.get(), &result[0], len)) {
        EXCEPTION("Failed to read Diffie Hellman public key");
    }

    return result;
}

struct Nounce {
    uint64_t salt = 0;
    uint64_t count = 0;
};

Crypto::Aes256::Aes256(SharedSecret secret, const uint64_t salt) : secret(std::move(secret)), salt(salt), counterEnc(1), counterDec(1) {
    if (this->secret.size() != 256/8) {
        EXCEPTION("Secret must be exactly 256 bits");
    }
}

Crypto::Aes256::~Aes256() = default;

static const unsigned char gcm_iv[16] = {
    0x99, 0xaa, 0x3e, 0x68, 0xed, 0x81, 0x73, 0xa0, 0xee, 0xd0, 0x66, 0x84, 0x01, 0x02, 0x03, 0x04,
};

size_t Crypto::Aes256::encrypt(const Span<unsigned char>& src, unsigned char* dst) {
    auto ctx = std::shared_ptr<EVP_CIPHER_CTX>(EVP_CIPHER_CTX_new(), [](EVP_CIPHER_CTX* p){ EVP_CIPHER_CTX_free(p); });

    Nounce nounce;
    //nounce.salt = static_cast<uint32_t>(salt & 0xffff) ^ static_cast<uint32_t>((salt & 0xffff0000) >> 32);
    nounce.salt = salt;
    nounce.count = counterEnc.fetch_add(1);
    const auto iv = reinterpret_cast<unsigned char*>(&nounce);

    if (!EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, secret.data(), iv)) {
        EXCEPTION("Failed to init decryption context");
    }

    int encryptLen;
    if (!EVP_EncryptUpdate(ctx.get(), dst, &encryptLen, src.data(), static_cast<int>(src.size()))) {
        EXCEPTION("Failed to encrypt data");
    }

    int finalizeLen;
    if (!EVP_EncryptFinal_ex(ctx.get(), dst + encryptLen, &finalizeLen)) {
        EXCEPTION("Failed to finalize encryption");
    }

    return encryptLen + finalizeLen;
}

size_t Crypto::Aes256::decrypt(const Span<unsigned char>& src, unsigned char* dst){
    auto ctx = std::shared_ptr<EVP_CIPHER_CTX>(EVP_CIPHER_CTX_new(), [](EVP_CIPHER_CTX* p){ EVP_CIPHER_CTX_free(p); });

    Nounce nounce;
    //nounce.salt = static_cast<uint32_t>(salt & 0xffff) ^ static_cast<uint32_t>((salt & 0xffff0000) >> 32);
    nounce.salt = salt;
    nounce.count = counterDec.fetch_add(1);
    const auto iv = reinterpret_cast<unsigned char*>(&nounce);

    if (!EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, secret.data(), iv)) {
        EXCEPTION("Failed to init decryption context");
    }

    int decryptLen;
    if (!EVP_DecryptUpdate(ctx.get(), dst, &decryptLen, src.data(), static_cast<int>(src.size()))) {
        EXCEPTION("Failed to decrypt data");
    }

    int finalizeLen;
    if (!EVP_DecryptFinal_ex(ctx.get(), dst + decryptLen, &finalizeLen)) {
        EXCEPTION("Failed to finalize decryption");
    }

    return decryptLen + finalizeLen;
}

size_t Crypto::Aes256::expectedEncryptSize(size_t length) {
    return length + EVP_MAX_BLOCK_LENGTH - 1;
}
