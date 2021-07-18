#pragma once

#include "../Math/Vector.hpp"
#include "../Utils/Msgpack.hpp"
#include "Mesh.hpp"

namespace Scissio {
enum class WireframeModel {
    None = 0,
    Box,
};

SCISSIO_API Mesh createSkyboxMesh();
SCISSIO_API Mesh createFullScreenMesh();
SCISSIO_API Mesh createWireframeMesh(WireframeModel type);
} // namespace Scissio

MSGPACK_ADD_ENUM(Scissio::WireframeModel);
