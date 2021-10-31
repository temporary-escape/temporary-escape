#pragma once

#include "../Network/NetworkStream.hpp"

namespace Scissio {
struct Session {
    Network::StreamPtr stream;
    uint64_t uid = 0;
};

using SessionPtr = std::shared_ptr<Session>;
} // namespace Scissio
