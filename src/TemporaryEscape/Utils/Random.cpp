#include "Random.hpp"
#include "NameGenerator.hpp"

#ifdef _WIN32
#include <rpc.h>
#else
#include <uuid/uuid.h>
#endif

using namespace Engine;

static NameGenerator names;

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

std::string Engine::randomName(std::mt19937_64& rng) {
    return names(rng);
}

std::string Engine::uuid() {
#ifdef _WIN32
    UUID uuid;
    UuidCreate(&uuid);
    char* str;
    UuidToStringA(&uuid, (RPC_CSTR*)&str);
    std::string res(str);
    RpcStringFreeA((RPC_CSTR*)&str);
    return res;
#else
    uuid_t id;
    uuid_generate(id);

    char data[37];
    uuid_unparse_lower(id, data);

    return std::string(data);
#endif
}
