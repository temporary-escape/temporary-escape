#include "Utils.hpp"

#include <iostream>

using namespace Scissio;

std::optional<Vector3> Scissio::intersectBox(const Vector3& min, const Vector3& max, const Vector3& from,
                                             const Vector3& to) {

    auto dir = to - from;
    const auto ml = glm::length(dir);
    dir = glm::normalize(dir);

    // Source of the algorithm: http://www.cs.utah.edu/~awilliam/box/
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    if (dir.x >= 0) {
        tmin = (min.x - from.x) / dir.x;
        tmax = (max.x - from.x) / dir.x;
    } else {
        tmin = (max.x - from.x) / dir.x;
        tmax = (min.x - from.x) / dir.x;
    }
    if (dir.y >= 0) {
        tymin = (min.y - from.y) / dir.y;
        tymax = (max.y - from.y) / dir.y;
    } else {
        tymin = (max.y - from.y) / dir.y;
        tymax = (min.y - from.y) / dir.y;
    }
    if ((tmin > tymax) || (tymin > tmax)) {
        return std::nullopt;
    }
    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }
    if (dir.z >= 0) {
        tzmin = (min.z - from.z) / dir.z;
        tzmax = (max.z - from.z) / dir.z;
    } else {
        tzmin = (max.z - from.z) / dir.z;
        tzmax = (min.z - from.z) / dir.z;
    }
    if ((tmin > tzmax) || (tzmin > tmax)) {
        return std::nullopt;
    }
    if (tzmin > tmin) {
        tmin = tzmin;
    }
    if (tzmax < tmax) {
        tmax = tzmax;
    }

    return from + dir * tmin;
}

static bool isAtSide(const float v) {
    return v >= 0.4999f && v <= 0.5001f;
}

Vector3 Scissio::intersectBoxNormal(const Vector3& center, const Vector3& pos) {
    const auto relative = pos - center;

    if (isAtSide(relative.x)) {
        return {1.0f, 0.0f, 0.0f};
    } else if (isAtSide(-relative.x)) {
        return {-1.0f, 0.0f, 0.0f};
    } else if (isAtSide(relative.y)) {
        return {0.0f, 1.0f, 0.0f};
    } else if (isAtSide(-relative.y)) {
        return {0.0f, -1.0f, 0.0f};
    } else if (isAtSide(relative.z)) {
        return {0.0f, 0.0f, 1.0f};
    } else {
        return {0.0f, 0.0f, -1.0f};
    }
}

Vector3 Scissio::screenToWorld(const Matrix4& viewMatrix, const Matrix4& projectionMatrix, const Vector2i& viewport,
                               const Vector2& pos) {
    const Vector4 rayClip((pos.x - 0) / float(viewport.x) * 2.0f - 1.0f,
                          -((pos.y - 0) / float(viewport.y) * 2.0f - 1.0f), 0.5f, 0.0f);

    const Vector4 rayEye = glm::inverse(projectionMatrix) * rayClip;

    const Vector4 rayEyeFixed(rayEye.x, rayEye.y, -1.0f, 0.0f);

    const auto rayWorld = viewMatrix * rayEyeFixed;
    return {rayWorld.x, rayWorld.y, rayWorld.z};
}

Vector2 Scissio::worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix, const Vector2i& viewport,
                               const Vector3& pos) {
    const auto vp = projectionMatrix * glm::inverse(viewMatrix);
    const auto clipSpace = vp * Vector4{pos, 1.0f};
    auto ndcSpace = Vector3{clipSpace} / clipSpace.w;
    ndcSpace = Vector3{ndcSpace.x, -ndcSpace.y, ndcSpace.z};
    return ((Vector2{ndcSpace} + Vector2{1.0f}) / 2.0f) * Vector2{viewport};
}
