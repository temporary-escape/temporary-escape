#pragma once

#include "../Network/NetworkStream.hpp"

namespace Scissio {
struct Session {
    Network::StreamPtr stream;
    std::string playerId;
};

using SessionPtr = std::shared_ptr<Session>;
} // namespace Scissio
