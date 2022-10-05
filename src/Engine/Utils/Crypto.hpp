#pragma once

#include "../Library.hpp"
#include "../Utils/Span.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace Engine::Crypto {
using SharedSecret = std::vector<unsigned char>;

using Key = std::string;

class ENGINE_API Ecdhe {
public:
    Ecdhe();
    ~Ecdhe();

    SharedSecret computeSharedSecret(const Key& other);
    Key getPublicKey();

private:
    struct Data;
    std::unique_ptr<Data> data;
};

class ENGINE_API Aes256 {
public:
    Aes256(SharedSecret secret, uint64_t salt);
    ~Aes256();

    template <typename Container> void encrypt(const Span<unsigned char>& src, Container& dst) {
        auto length = expectedEncryptSize(src.size());
        dst.resize(length);
        length = encrypt(src, dst.data());
        dst.resize(length);
    }

    template <typename Container> void decrypt(const Span<unsigned char>& src, Container& dst) {
        dst.resize(src.size());
        const auto length = decrypt(src, dst.data());
        dst.resize(length);
    }

    size_t encrypt(const Span<unsigned char>& src, unsigned char* dst);
    size_t decrypt(const Span<unsigned char>& src, unsigned char* dst);
    size_t expectedEncryptSize(size_t length);

private:
    SharedSecret secret;
    uint64_t salt;
    std::atomic<uint64_t> counterEnc;
    std::atomic<uint64_t> counterDec;
};
} // namespace Engine::Crypto
