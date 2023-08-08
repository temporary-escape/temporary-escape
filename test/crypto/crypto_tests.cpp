#include "../common.hpp"
#include <engine/crypto/diffie_hellman_key.hpp>
#include <engine/crypto/private_key.hpp>
#include <engine/crypto/x509_cert.hpp>
#include <iostream>

using namespace Engine;

TEST_CASE("Generate random DH params and then load it back", "[crypto]") {
    DiffieHellmanKey ec{};
    const auto& pem = ec.pem();
    REQUIRE(pem.empty() == false);
    REQUIRE(pem.find("BEGIN DH PARAMETERS") != std::string::npos);
    REQUIRE(pem.front() == '-');
    REQUIRE(pem.at(pem.size() - 2) == '-');
    REQUIRE(pem.back() == '\n');

    REQUIRE_NOTHROW(DiffieHellmanKey{pem});

    REQUIRE(DiffieHellmanKey{pem}.pem() == pem);
}

TEST_CASE("Generate random private key and then load it back", "[crypto]") {
    PrivateKey key{};
    const auto& pem = key.pem();
    REQUIRE(pem.empty() == false);
    REQUIRE(pem.find("BEGIN RSA PRIVATE KEY") != std::string::npos);
    REQUIRE(pem.front() == '-');
    REQUIRE(pem.at(pem.size() - 2) == '-');
    REQUIRE(pem.back() == '\n');

    REQUIRE_NOTHROW(PrivateKey{pem});

    REQUIRE(PrivateKey{pem}.pem() == pem);
}

TEST_CASE("Generate random certificate and then load it back", "[crypto]") {
    PrivateKey key{};
    X509Cert cert{key};

    const auto& pem = cert.pem();
    REQUIRE(pem.empty() == false);
    REQUIRE(pem.find("BEGIN CERTIFICATE") != std::string::npos);
    REQUIRE(pem.front() == '-');
    REQUIRE(pem.at(pem.size() - 2) == '-');
    REQUIRE(pem.back() == '\n');

    REQUIRE_NOTHROW(X509Cert{pem});

    REQUIRE(X509Cert{pem}.pem() == pem);
}
