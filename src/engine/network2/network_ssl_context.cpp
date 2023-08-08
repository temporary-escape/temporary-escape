#include "network_ssl_context.hpp"

using namespace Engine;

NetworkSslContext::NetworkSslContext() : ssl{asio::ssl::context::tlsv13} {
    ssl.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |
                    asio::ssl::context::no_sslv3 | asio::ssl::context::no_tlsv1_1 | asio::ssl::context::no_tlsv1_2 |
                    asio::ssl::context::single_dh_use | asio::ssl::context::no_compression);
}

NetworkSslContext::~NetworkSslContext() = default;

void NetworkSslContext::setVerifyNone() {
    ssl.set_verify_mode(asio::ssl::verify_none);
}

void NetworkSslContext::setVerify(std::function<bool(X509Cert)> callback) {
    ssl.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    ssl.set_verify_callback([callback = std::move(callback)](const bool preVerified, asio::ssl::verify_context& ctx) {
        (void)preVerified;
        return callback(X509Cert{ctx});
    });
}

void NetworkSslContext::setPrivateKey(const PrivateKey& pkey) {
    ssl.use_private_key(asio::buffer(pkey.pem()), asio::ssl::context::pem);
}

void NetworkSslContext::setDiffieHellmanKey(const DiffieHellmanKey& ec) {
    ssl.use_tmp_dh(asio::buffer(ec.pem()));
}

void NetworkSslContext::setCertificate(const X509Cert& cert) {
    ssl.use_certificate_chain(asio::buffer(cert.pem()));
}
