#pragma once

#define ASIO_STANDALONE

#include "private_key.hpp"
#include <asio.hpp>
#include <asio/ssl.hpp>

// Forward declaration
struct x509_st;
typedef struct x509_st X509;

namespace Engine {
class ENGINE_API X509Cert {
public:
    static std::string certFromAsioContext(asio::ssl::verify_context& ctx);

    explicit X509Cert(std::string raw);
    explicit X509Cert(const PrivateKey& pkey);
    explicit X509Cert(asio::ssl::verify_context& ctx);
    ~X509Cert();

    [[nodiscard]] const std::string& pem() const {
        return raw;
    }

    [[nodiscard]] X509* get() const {
        return x509.get();
    }

    std::string getSubjectName() const;

private:
    std::shared_ptr<X509> x509;
    std::string raw{};
};
} // namespace Engine
