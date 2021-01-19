#include "Utils.hpp"

using namespace Scissio;

std::optional<Vector3> Scissio::intersectBox(const Vector3& min, const Vector3& max, const Vector3& from,
                                             const Vector3& to) {

    auto dir = to - from;
    const auto ml = glm::length(dir);
    dir = glm::normalize(dir);

    /*float t1 = (min.x - from.x) / dir.x;
    float t2 = (max.x - from.x) / dir.x;
    float t3 = (min.y - from.y) / dir.y;
    float t4 = (max.y - from.y) / dir.y;
    float t5 = (min.z - from.z) / dir.z;
    float t6 = (max.z - from.z) / dir.z;

    float aMin = t1 < t2 ? t1 : t2;
    float bMin = t3 < t4 ? t3 : t4;
    float cMin = t5 < t6 ? t5 : t6;

    float aMax = t1 > t2 ? t1 : t2;
    float bMax = t3 > t4 ? t3 : t4;
    float cMax = t5 > t6 ? t5 : t6;

    float fMax = aMin > bMin ? aMin : bMin;
    float fMin = aMax < bMax ? aMax : bMax;

    float t7 = fMax > cMin ? fMax : cMin;
    float t8 = fMin < cMax ? fMin : cMax;

    float t9 = (t8 < 0 || t7 > t8) ? -1 : t7;

    if (t9 > ml) {
        return std::nullopt;
    }*/

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

Vector3 Scissio::screenToWorld(const Matrix4& viewMatrix, const Matrix4& projectionMatrix, const Vector2i& viewport,
                               const Vector2i& pos) {
    const Vector4 rayClip((pos.x - 0) / float(viewport.x) * 2.0f - 1.0f,
                          -((pos.y - 0) / float(viewport.y) * 2.0f - 1.0f), 0.5f, 0.0f);

    const Vector4 rayEye = projectionMatrix * rayClip;

    const Vector4 rayEyeFixed(rayEye.x, rayEye.y, -1.0f, 0.0f);

    const auto rayWorld = viewMatrix * rayEyeFixed;
    return {rayWorld.x, rayWorld.y, rayWorld.z};
}
