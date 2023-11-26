#pragma once

#include "../Crypto/DiffieHellmanKey.hpp"
#include "../Crypto/PrivateKey.hpp"
#include "../Crypto/X509Cert.hpp"
#include <asio/ssl.hpp>

namespace Engine {
class ENGINE_API NetworkSslContext {
public:
    NetworkSslContext();
    virtual ~NetworkSslContext();

    void setVerify(std::function<bool(X509Cert)> callback);
    void setVerifyNone();
    void setPrivateKey(const PrivateKey& pkey);
    void setDiffieHellmanKey(const DiffieHellmanKey& ec);
    void setCertificate(const X509Cert& cert);

    asio::ssl::context& get() {
        return ssl;
    }

    const asio::ssl::context& get() const {
        return ssl;
    }

private:
    asio::ssl::context ssl;
};
} // namespace Engine
