#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "DiffieHellmanKey.hpp"
#include <memory>
#include <openssl/dh.h>
#include <openssl/pem.h>
#include <stdexcept>

using namespace Engine;

DiffieHellmanKey::DiffieHellmanKey(std::string raw) : raw{std::move(raw)} {
}

DiffieHellmanKey::DiffieHellmanKey() {
    DH* dh = DH_get_2048_256();

    auto bio = std::shared_ptr<BIO>(BIO_new(BIO_s_mem()), [](BIO* b) { BIO_free(b); });

    if (!PEM_write_bio_DHparams(bio.get(), dh)) {
        throw std::runtime_error("Failed to write BIO for the ECDHE key");
    }

    const auto len = BIO_pending(bio.get());
    raw.resize(len);
    if (!BIO_read(bio.get(), raw.data(), len)) {
        throw std::runtime_error("Failed to read Diffie Hellman private key");
    }
}

DiffieHellmanKey::~DiffieHellmanKey() = default;

#pragma clang diagnostic pop
