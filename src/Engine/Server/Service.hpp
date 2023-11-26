#pragma once

#include "PlayerSessions.hpp"
#include "Schemas.hpp"

namespace Engine {
class ENGINE_API Service {
public:
    Service() : hashCode{typeid(*this).hash_code()} {
    }
    virtual ~Service() = 0;

    uint64_t getType() const {
        return hashCode;
    }

private:
    uint64_t hashCode;
};

inline Service::~Service() {
}
} // namespace Engine
