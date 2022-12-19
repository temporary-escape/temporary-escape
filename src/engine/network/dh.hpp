#pragma once

#include "../library.hpp"
#include <memory>
#include <string>

// Forward definition
struct evp_pkey_st;
typedef struct evp_pkey_st EVP_PKEY;

namespace Engine::Network {
class ENGINE_API Dh {
public:
    explicit Dh(std::string raw);
    Dh();
    ~Dh();

    [[nodiscard]] const std::string& pem() const {
        return raw;
    }

private:
    std::string raw{};
};
} // namespace Engine::Network
