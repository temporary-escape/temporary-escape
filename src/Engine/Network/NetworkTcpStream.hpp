#pragma once

#include "../Library.hpp"
#include "../Utils/Crypto.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "NetworkAsio.hpp"
#include "Packet.hpp"

namespace Engine {
template <typename Sink>
class ENGINE_API NetworkTcpStream : public std::enable_shared_from_this<NetworkTcpStream<Sink>> {
public:
    explicit NetworkTcpStream(Crypto::Ecdhe& ecdhe, asio::ip::tcp::socket socket)
        : ecdhe(ecdhe), socket(std::move(socket)) {
    }

    void start() {
        receive();
    }

    void close() {
        if (socket.is_open()) {
            // Log::w(CMP, "Closing connection {}:{}", socket.remote_endpoint().address().to_string(),
            //        socket.remote_endpoint().port());
            socket.close();
        }
    }

    void sendPublicKey() {
        Packet packet;
        const auto pkey = ecdhe.getPublicKey();
        packet.data.write(pkey.data(), pkey.size());
        packet.id = 0;
        sendRaw(packet);
    }

    void sendRaw(const Packet& packet) {
        auto sbuffer = std::make_shared<msgpack::sbuffer>();
        msgpack::pack(*sbuffer, packet);

        auto self = NetworkTcpStream<Sink>::shared_from_this();
        const auto b = asio::buffer(sbuffer->data(), sbuffer->size());
        self->socket.async_write_some(b, [self, sbuffer](const asio::error_code ec, const size_t length) {
            (void)self;

            if (ec) {
                Log::e(CMP, "async_write_some error: {}", ec.message());
            }
        });
    }

    template <typename T> void send(const T& message) {
        Packet packet;
        msgpack::pack(packet.data, message);
        packet.id = Sink::template indexOf<T>();
        encrypt(packet);
        sendRaw(packet);
    }

private:
    static inline const char* CMP = "NetworkTcpStream";
    static constexpr int WINDOW_SIZE = 1024 * 32;

    virtual void onConnected() = 0;
    virtual void onReceive(Packet packet) = 0;

    void receive() {
        unp.reserve_buffer(WINDOW_SIZE);
        const auto b = asio::buffer(unp.buffer(), WINDOW_SIZE);
        auto self = NetworkTcpStream<Sink>::shared_from_this();
        self->socket.async_read_some(b, [self](const asio::error_code ec, const size_t length) {
            if (ec) {
                Log::e(CMP, "async_read_some error: {}", ec.message());

                if (ec == asio::error::eof || ec == asio::error::connection_reset) {
                    /*if (self->flag.load()) {
                        self->disconnect();
                        self->acceptor.eventDisconnect(self);
                    }*/
                }

                self->close();
            } else {
                self->unp.buffer_consumed(length);
                msgpack::object_handle oh;
                while (self->unp.next(oh)) {
                    try {
                        Packet packet;
                        oh.get().convert(packet);
                        // Log::d(CMP, "Network TCP stream accepted packet id: {}", packet.id);

                        if (packet.id == 0) {
                            self->acceptPublicKey(packet);
                            self->onConnected();
                        } else {
                            self->decrypt(packet);
                            self->onReceive(std::move(packet));
                        }
                    } catch (std::exception& e) {
                        BACKTRACE(CMP, e, "Error accepting packet");
                    }
                }

                self->receive();
            }
        });
    }

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

        Log::i(CMP, "Public key accepted from: {}:{}", socket.remote_endpoint().address().to_string(),
               socket.remote_endpoint().port());
    }

    void encrypt(Packet& packet) {
        if (!aes) {
            EXCEPTION("AES-256 has not been initialized");
        } else {
            PacketBuffer enc;
            enc.resize(aes->expectedEncryptSize(packet.data.size()));
            aes->encrypt(packet.data, enc);

            swap(enc, packet.data);
        }
    }

    void decrypt(Packet& packet) {
        if (!aes) {
            EXCEPTION("AES-256 has not been initialized");
        } else {
            PacketBuffer dec;
            dec.resize(packet.data.size());

            aes->decrypt(packet.data, dec);

            swap(dec, packet.data);
        }
    }

    Crypto::Ecdhe& ecdhe;
    asio::ip::tcp::socket socket;
    msgpack::unpacker unp;
    std::unique_ptr<Crypto::Aes256> aes;
};
} // namespace Engine
