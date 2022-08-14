#pragma once

#include "../Graphics/Mesh.hpp"
#include "Material.hpp"

namespace Engine {
struct Primitive {
    VertexBuffer vbo{NO_CREATE};
    VertexBuffer ibo{NO_CREATE};
    VertexBuffer ubo{NO_CREATE};
    Mesh mesh{NO_CREATE};
    Material material;
};
} // namespace Engine
