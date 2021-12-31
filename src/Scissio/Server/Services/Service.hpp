#pragma once

#include "../../Assets/AssetManager.hpp"
#include "../../Config.hpp"
#include "../Database.hpp"
#include "../Schemas.hpp"

namespace Scissio {
class Service {
public:
    virtual ~Service() = default;
    virtual void tick() = 0;
};
} // namespace Scissio
