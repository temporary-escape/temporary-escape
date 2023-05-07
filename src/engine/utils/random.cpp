#include "random.hpp"
#include "name_generator.hpp"
#include <array>

#if defined(_WIN32)
#include <rpc.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFUUID.h>
#else
#include <uuid/uuid.h>
#endif

using namespace Engine;

uint64_t Engine::randomId(const std::function<bool(uint64_t)>& pred) {
    union RandomUnion {
        uint8_t bytes[8];
        int64_t value;
    };

    std::random_device r;
    std::mt19937_64 rng{r()};
    std::uniform_int_distribution<int> dist(0, 0xFF);

    RandomUnion id{};
    while (true) {
        for (auto i = 0; i < 8; i++) {
            id.bytes[i] = dist(rng);
        }

        if (!pred || pred(id.value)) {
            break;
        }
    }

    return id.value;
}

#if defined(__APPLE__)
static std::array<char, 16> hexChars = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static char hexHigh(const uint8_t b) {
    return hexChars[(b & 0xF0) >> 4];
}
static char hexLow(const uint8_t b) {
    return hexChars[b & 0x0F];
}
#endif

std::string Engine::uuid() {
#if defined(_WIN32)
    UUID uuid;
    UuidCreate(&uuid);
    char* str;
    UuidToStringA(&uuid, (RPC_CSTR*)&str);
    std::string res(str);
    RpcStringFreeA((RPC_CSTR*)&str);
    return res;
#elif defined(__APPLE__)
    auto newId = CFUUIDCreate(NULL);
    auto bytes = CFUUIDGetUUIDBytes(newId);
    CFRelease(newId);
    std::string res;
    res.resize(36);
    res[0] = hexHigh(bytes.byte0);
    res[1] = hexLow(bytes.byte0);
    res[2] = hexHigh(bytes.byte1);
    res[3] = hexLow(bytes.byte1);
    res[4] = hexHigh(bytes.byte2);
    res[5] = hexLow(bytes.byte2);
    res[6] = hexHigh(bytes.byte3);
    res[7] = hexLow(bytes.byte3);
    res[8] = '-';
    res[9] = hexHigh(bytes.byte4);
    res[10] = hexLow(bytes.byte4);
    res[11] = hexHigh(bytes.byte5);
    res[12] = hexLow(bytes.byte5);
    res[13] = '-';
    res[14] = hexHigh(bytes.byte6);
    res[15] = hexLow(bytes.byte6);
    res[16] = hexHigh(bytes.byte7);
    res[17] = hexLow(bytes.byte7);
    res[18] = '-';
    res[19] = hexHigh(bytes.byte8);
    res[20] = hexLow(bytes.byte8);
    res[21] = hexHigh(bytes.byte9);
    res[22] = hexLow(bytes.byte9);
    res[23] = '-';
    res[24] = hexHigh(bytes.byte10);
    res[25] = hexLow(bytes.byte10);
    res[26] = hexHigh(bytes.byte11);
    res[27] = hexLow(bytes.byte11);
    res[28] = hexHigh(bytes.byte12);
    res[29] = hexLow(bytes.byte12);
    res[30] = hexHigh(bytes.byte13);
    res[31] = hexLow(bytes.byte13);
    res[32] = hexHigh(bytes.byte14);
    res[33] = hexLow(bytes.byte14);
    res[34] = hexHigh(bytes.byte15);
    res[35] = hexLow(bytes.byte15);
    return res;
#else
    uuid_t id;
    uuid_generate(id);

    char data[37];
    uuid_unparse_lower(id, data);

    return std::string(data);
#endif
}
