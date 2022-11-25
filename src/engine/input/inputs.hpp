#pragma once

#include "../library.hpp"

namespace Engine {
enum class Input {
    None = 0,
    GuiToggleGalaxyMap,
    PointerMovement,
    CameraFreeLookForward,
    CameraFreeLookBackwards,
    CameraFreeLookLeft,
    CameraFreeLookRight,
    CameraFreeLookUp,
    CameraFreeLookDown,
    CameraFreeLookRotation,
    CameraFreeLookFast,
    CameraPan,
    CameraZoomIn,
    CameraZoomOut,
};
}
