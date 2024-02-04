#include "../../Common.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include <Engine/Crypto/AES.hpp>
#include <Engine/Crypto/ECDH.hpp>

using namespace Engine;

TEST_CASE("Generate ECDH keys and derive shared secret", "[crypto]") {
    ECDH ecdhA{};
    const auto pkeyA = ecdhA.getPublicKey();
    REQUIRE(pkeyA.find("-----BEGIN PUBLIC KEY-----") == 0);
    REQUIRE(pkeyA.find("-----END PUBLIC KEY-----") != std::string::npos);

    ECDH ecdhB{};
    const auto pkeyB = ecdhB.getPublicKey();
    REQUIRE(pkeyB.find("-----BEGIN PUBLIC KEY-----") == 0);
    REQUIRE(pkeyB.find("-----END PUBLIC KEY-----") != std::string::npos);

    REQUIRE(pkeyA != pkeyB);

    const auto secretA = ecdhA.deriveSharedSecret(pkeyB);
    REQUIRE(secretA.size() == 48);

    const auto secretB = ecdhB.deriveSharedSecret(pkeyA);
    REQUIRE(secretB.size() == 48);

    REQUIRE(std::memcmp(secretA.data(), secretB.data(), secretA.size()) == 0);

    AES aesA{secretA};
    AES aesB{secretB};

    // Encryption step
    std::string msg = "Hello World from temporary escape unit tests!";
    std::vector<char> res0;
    res0.resize(AES::getEncryptSize(msg.size()));
    std::vector<char> res1;
    res1.resize(res0.size());

    // Encrypt the message
    auto out = aesA.encrypt(msg.data(), res0.data(), msg.size());
    REQUIRE(out == res0.size());

    // Encrypt it again
    out = aesA.encrypt(msg.data(), res1.data(), msg.size());
    REQUIRE(out == res1.size());

    // Must not be equal
    REQUIRE(std::memcmp(res0.data(), res1.data(), res0.size()) != 0);

    // Decryption step
    std::vector<char> res2;
    res2.resize(AES::getDecryptSize(res0.size()));
    REQUIRE(res2.size() < res0.size());

    // Decrypt the message
    out = aesB.decrypt(res0.data(), res2.data(), res0.size());
    REQUIRE(out == res2.size());
    REQUIRE(std::string{res2.data(), res2.size()} == msg);

    // Decrypt the message again
    out = aesB.decrypt(res0.data(), res2.data(), res0.size());
    REQUIRE(out == res2.size());
    REQUIRE(std::string{res2.data(), res2.size()} == msg);

    // Decrypt the second message
    out = aesB.decrypt(res1.data(), res2.data(), res1.size());
    REQUIRE(out == res2.size());
    REQUIRE(std::string{res2.data(), res2.size()} == msg);
}
