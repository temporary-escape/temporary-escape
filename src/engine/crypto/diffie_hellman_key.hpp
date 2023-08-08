#pragma once

#include "../library.hpp"
#include <memory>
#include <string>

// Forward definition
struct evp_pkey_st;
typedef struct evp_pkey_st EVP_PKEY;

namespace Engine {
class ENGINE_API DiffieHellmanKey {
public:
    explicit DiffieHellmanKey(std::string raw);
    DiffieHellmanKey();
    ~DiffieHellmanKey();

    [[nodiscard]] const std::string& pem() const {
        return raw;
    }

private:
    std::string raw{};
};
} // namespace Engine
