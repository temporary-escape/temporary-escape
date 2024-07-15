#pragma once

#include "../Library.hpp"
#include <memory>
#include <string>
#include <vector>

// Forward declaration
typedef struct evp_pkey_st EVP_PKEY;

namespace Engine {
class ENGINE_API ECDH {
public:
    explicit ECDH(const std::string_view& curve = "secp384r1");
    ~ECDH();

    static bool isPublicKey(const std::string_view& other);

    std::string getPublicKey() const;
    std::vector<uint8_t> deriveSharedSecret(const std::string_view& other);

private:
    static void evpDeleter(EVP_PKEY* p);
    using EvpKeyPtr = std::unique_ptr<EVP_PKEY, decltype(&evpDeleter)>;

    EvpKeyPtr evp;
};
} // namespace Engine
