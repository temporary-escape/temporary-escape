#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include <vector>

namespace Scissio {
extern SCISSIO_API std::vector<Vector4i> packRectangles(const Vector2i& maxSize, const std::vector<Vector2i>& items,
                                                        int offset);
}
