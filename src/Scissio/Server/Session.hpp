#pragma once

#include "../Network/NetworkStream.hpp"

namespace Scissio {
struct Session {
    Network::StreamPtr stream;
    std::string playerId;
    std::string sectorCompoundId;
};

using SessionPtr = std::shared_ptr<Session>;
} // namespace Scissio
