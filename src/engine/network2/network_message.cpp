#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "network_message.hpp"
#include <openssl/sha.h>

using namespace Engine;

uint64_t Detail::getMessageHash(const std::string_view& name) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, name.data(), name.size());
    SHA256_Final(hash, &sha256);

    // Only convert the first part
    return *reinterpret_cast<const uint64_t*>(hash);
}

#pragma clang diagnostic pop

bool Detail::validateMessageObject(const msgpack::object_handle& oh) {
    return oh.zone() && oh->type == msgpack::type::ARRAY && oh->via.array.size == 3;
}
