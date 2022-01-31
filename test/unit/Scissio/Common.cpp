#include "Common.hpp"

struct Init {
    Init() {
        Log::configure(true);
    }
};

static Init init{};
