#pragma once

#include "../Utils/Crypto.hpp"
#include "Packet.hpp"

namespace Engine::Network {
class ENGINE_API Stream {
public:
    explicit Stream(Crypto::Ecdhe& ecdhe) : ecdhe(ecdhe) {
    }
    virtual ~Stream() = default;

    virtual void sendRaw(const Packet& packet) = 0;

    template <typename T> void send(const T& message) {
        Packet packet;
        msgpack::pack(packet.data, message);
        packet.id = getMessageId<T>();
        encrypt(packet);
        sendRaw(packet);
    }

    void sendPublicKey() {
        Packet packet;
        const auto pkey = ecdhe.getPublicKey();
        packet.data.write(pkey.data(), pkey.size());
        packet.id = 0;
        sendRaw(packet);
    }

protected:
    void acceptPublicKey(Packet& packet) {
        if (aes) {
            EXCEPTION("AES-256 has already been initialized");
        }

        if (packet.id != 0) {
            EXCEPTION("First received packet must be a public key");
        }

        Crypto::Key pkey(reinterpret_cast<const char*>(packet.data.data()), packet.data.size());
        const auto sharedSecret = ecdhe.computeSharedSecret(pkey);

        const auto salt = reinterpret_cast<const uint64_t*>(sharedSecret.data());

        aes = std::make_unique<Crypto::Aes256>(sharedSecret, *salt);
    }

    void encrypt(Packet& packet) {
        if (!aes) {
            EXCEPTION("AES-256 has not been initialized");
        } else {
            decltype(Packet::data) enc;
            enc.resize(aes->expectedEncryptSize(packet.data.size()));
            aes->encrypt(packet.data, enc);

            swap(enc, packet.data);
        }
    }

    void decrypt(Packet& packet) {
        if (!aes) {
            EXCEPTION("AES-256 has not been initialized");
        } else {
            decltype(Packet::data) dec;
            dec.resize(packet.data.size());

            aes->decrypt(packet.data, dec);

            swap(dec, packet.data);
        }
    }

private:
    Crypto::Ecdhe& ecdhe;
    std::unique_ptr<Crypto::Aes256> aes;
};

using StreamPtr = std::shared_ptr<Stream>;
} // namespace Engine::Network
