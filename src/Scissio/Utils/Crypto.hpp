#pragma once

#include "../Library.hpp"
#include "../Utils/Span.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace Scissio::Crypto {
using SharedSecret = std::vector<unsigned char>;
using Buffer = std::vector<unsigned char>;

using Key = std::string;

class SCISSIO_API Ecdhe {
public:
    Ecdhe();
    ~Ecdhe();

    SharedSecret computeSharedSecret(const Key& other);
    Key getPublicKey();

private:
    struct Data;
    std::unique_ptr<Data> data;
};

class SCISSIO_API Aes256 {
public:
    Aes256(SharedSecret secret, uint64_t salt);
    ~Aes256();

    Buffer encrypt(const Span<unsigned char>& data);
    Buffer decrypt(const Span<unsigned char>& data);

private:
    SharedSecret secret;
    uint64_t salt;
    std::atomic<uint64_t> counterEnc;
    std::atomic<uint64_t> counterDec;
};
} // namespace Scissio::Crypto
