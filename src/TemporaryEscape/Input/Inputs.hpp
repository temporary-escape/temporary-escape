#pragma once

#include "../Library.hpp"

namespace Engine {
enum class Input {
    None = 0,
    CameraFreeLookForward,
    CameraFreeLookBackwards,
    CameraFreeLookLeft,
    CameraFreeLookRight,
    CameraFreeLookRotation,
};
}
