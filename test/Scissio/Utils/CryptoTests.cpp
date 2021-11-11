#include "../Common.hpp"
#include <Utils/Crypto.hpp>

#define TAG "[Crypto]"

TEST_CASE("Generate ECDHE and compute shared", TAG) {
    Crypto::Ecdhe alice;
    Crypto::Ecdhe bob;

    const auto pubKeyAlice = alice.getPublicKey();
    REQUIRE(!pubKeyAlice.empty());
    REQUIRE(pubKeyAlice.find("-----BEGIN PUBLIC KEY-----") == 0);

    const auto pubKeyBob = bob.getPublicKey();
    REQUIRE(!pubKeyBob.empty());
    REQUIRE(pubKeyBob.find("-----BEGIN PUBLIC KEY-----") == 0);

    const auto sharedSecretAlice = alice.computeSharedSecret(pubKeyBob);
    const auto sharedSecretBob = bob.computeSharedSecret(pubKeyAlice);

    REQUIRE(!sharedSecretAlice.empty());
    REQUIRE(!sharedSecretBob.empty());

    REQUIRE(sharedSecretAlice.size() == sharedSecretBob.size());

    const auto res = std::memcmp(sharedSecretAlice.data(), sharedSecretBob.data(), sharedSecretAlice.size());
    REQUIRE(res == 0);
}

TEST_CASE("Encrypt and decrypt via AES-256", TAG) {
    Crypto::SharedSecret secret = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x08, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x18, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    };
    uint64_t seed = 0x1234567812345678;

    Crypto::Aes256 alice(secret, seed);
    Crypto::Aes256 bob(secret, seed);

    std::string msg = "This is a message from Alice";
    std::vector<unsigned char> enc;
    std::vector<unsigned char> dec;

    alice.encrypt({reinterpret_cast<const unsigned char*>(msg.data()), msg.size()}, enc);
    REQUIRE(enc.size() >= msg.size());

    bob.decrypt(enc, dec);
    REQUIRE(dec.size() == msg.size());

    std::string decStr(reinterpret_cast<const char*>(dec.data()), dec.size());
    REQUIRE(decStr == msg);

    msg = "This is a message from Bob";

    bob.encrypt({reinterpret_cast<const unsigned char*>(msg.data()), msg.size()}, enc);
    REQUIRE(enc.size() >= msg.size());

    alice.decrypt(enc, dec);
    REQUIRE(dec.size() == msg.size());

    decStr = std::string(reinterpret_cast<const char*>(dec.data()), dec.size());
    REQUIRE(decStr == msg);

    msg = "Alice says good bye Bob!";

    alice.encrypt({reinterpret_cast<const unsigned char*>(msg.data()), msg.size()}, enc);
    REQUIRE(enc.size() >= msg.size());

    bob.decrypt(enc, dec);
    REQUIRE(dec.size() == msg.size());

    decStr = std::string(reinterpret_cast<const char*>(dec.data()), dec.size());
    REQUIRE(decStr == msg);
}
