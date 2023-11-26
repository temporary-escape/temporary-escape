#pragma once

#include "../Library.hpp"
#include <memory>
#include <string>

// Forward definition
struct evp_pkey_st;
typedef struct evp_pkey_st EVP_PKEY;

namespace Engine {
class ENGINE_API PrivateKey {
public:
    explicit PrivateKey(std::string raw);
    PrivateKey();
    ~PrivateKey();

    [[nodiscard]] const std::string& pem() const {
        return raw;
    }

    [[nodiscard]] EVP_PKEY* get() const {
        return pkey.get();
    }

private:
    std::shared_ptr<EVP_PKEY> pkey;
    std::string raw{};
};
} // namespace Engine
