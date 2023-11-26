#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "Md5.hpp"
#include <openssl/md5.h>

using namespace Engine;

std::string Engine::md5sum(const void* data, size_t size) {
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    unsigned char result[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data), size, result);

    std::string str;
    str.resize(MD5_DIGEST_LENGTH * 2);
    for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
        str[i * 2 + 0] = hex[((result[i] & 0xF0) >> 4)];
        str[i * 2 + 1] = hex[(result[i] & 0x0F)];
    }
    return str;
}

#pragma clang diagnostic pop
