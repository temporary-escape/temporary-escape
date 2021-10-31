#pragma once

#include "TextureCubemap.hpp"

namespace Scissio {
struct Skybox {
    TextureCubemap texture{NO_CREATE};
    TextureCubemap prefilter{NO_CREATE};
    TextureCubemap irradiance{NO_CREATE};
};
} // namespace Scissio
