#pragma once

#include "../Graphics/TextureCubemap.hpp"

namespace Engine {
struct Skybox {
    TextureCubemap texture{NO_CREATE};
    TextureCubemap prefilter{NO_CREATE};
    TextureCubemap irradiance{NO_CREATE};
};
} // namespace Engine
