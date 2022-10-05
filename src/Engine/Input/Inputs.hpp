#pragma once

#include "../Library.hpp"

namespace Engine {
enum class Input {
    None = 0,
    PointerMovement,
    CameraFreeLookForward,
    CameraFreeLookBackwards,
    CameraFreeLookLeft,
    CameraFreeLookRight,
    CameraFreeLookUp,
    CameraFreeLookDown,
    CameraFreeLookRotation,
    CameraFreeLookFast,
};
}
