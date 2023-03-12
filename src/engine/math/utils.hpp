#pragma once

#include "../library.hpp"
#include "matrix.hpp"
#include "vector.hpp"

#include <optional>

namespace Engine {
extern ENGINE_API std::optional<Vector3> intersectBox(const Vector3& min, const Vector3& max, const Vector3& from,
                                                      const Vector3& to);
extern ENGINE_API Vector3 intersectBoxNormal(const Vector3& center, const Vector3& pos);
extern ENGINE_API Vector3 screenToWorld(const Matrix4& viewMatrix, const Matrix4& projectionMatrix,
                                        const Vector2i& viewport, const Vector2& pos);
extern ENGINE_API Vector2 worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix,
                                        const Vector2i& viewport, const Vector3& pos, bool invert = false);
extern ENGINE_API std::vector<Vector2> worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix,
                                                     const Vector2i& viewport, const std::vector<Vector3>& positions,
                                                     bool invert = false);
extern ENGINE_API std::vector<double> gaussianKernel(size_t size = 5, double sigma = 1.0, double mu = 0.0,
                                                     double step = 1.0);

template <typename T> inline T map(T x, T inMin, T inMax, T outMin, T outMax) {
    return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

template <typename T> inline T lerp(T a, T b, T f) {
    return a + f * (b - a);
}

extern ENGINE_API Color4 fromRgbBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
extern ENGINE_API Color4 hsvToRgb(const Vector4& in);
} // namespace Engine
