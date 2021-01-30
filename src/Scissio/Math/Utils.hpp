#pragma once

#include "../Library.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

#include <optional>

namespace Scissio {
extern SCISSIO_API std::optional<Vector3> intersectBox(const Vector3& min, const Vector3& max, const Vector3& from,
                                                       const Vector3& to);
extern SCISSIO_API Vector3 intersectBoxNormal(const Vector3& center, const Vector3& pos);
extern SCISSIO_API Vector3 screenToWorld(const Matrix4& viewMatrix, const Matrix4& projectionMatrix,
                                         const Vector2i& viewport, const Vector2i& pos);
} // namespace Scissio