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
                                         const Vector2i& viewport, const Vector2& pos);
extern SCISSIO_API Vector2 worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix,
                                         const Vector2i& viewport, const Vector3& pos);

template <typename T> inline T map(T x, T in_min, T in_max, T out_min, T out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
} // namespace Scissio
