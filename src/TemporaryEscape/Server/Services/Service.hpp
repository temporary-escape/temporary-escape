#pragma once

#include "../../Assets/AssetManager.hpp"
#include "../../Config.hpp"
#include "../../Utils/Database.hpp"
#include "../Schemas.hpp"

namespace Engine {
class ENGINE_API Service {
public:
    virtual ~Service() = default;
    virtual void tick() = 0;
};
} // namespace Engine
