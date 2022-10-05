#include "Utils.hpp"

#include <iostream>

using namespace Engine;

std::optional<Vector3> Engine::intersectBox(const Vector3& min, const Vector3& max, const Vector3& from,
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

Vector3 Engine::intersectBoxNormal(const Vector3& center, const Vector3& pos) {
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

Vector3 Engine::screenToWorld(const Matrix4& viewMatrix, const Matrix4& projectionMatrix, const Vector2i& viewport,
                              const Vector2& pos) {
    const Vector4 rayClip((pos.x - 0) / float(viewport.x) * 2.0f - 1.0f,
                          -((pos.y - 0) / float(viewport.y) * 2.0f - 1.0f), 0.5f, 0.0f);

    const Vector4 rayEye = glm::inverse(projectionMatrix) * rayClip;

    const Vector4 rayEyeFixed(rayEye.x, rayEye.y, -1.0f, 0.0f);

    const auto rayWorld = viewMatrix * rayEyeFixed;
    return {rayWorld.x, rayWorld.y, rayWorld.z};
}

Vector2 Engine::worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix, const Vector2i& viewport,
                              const Vector3& pos) {
    const auto vp = projectionMatrix * glm::inverse(viewMatrix);
    const auto clipSpace = vp * Vector4{pos, 1.0f};
    auto ndcSpace = Vector3{clipSpace} / clipSpace.w;
    ndcSpace = Vector3{ndcSpace.x, -ndcSpace.y, ndcSpace.z};
    return ((Vector2{ndcSpace} + Vector2{1.0f}) / 2.0f) * Vector2{viewport};
}

// Adapted from https://github.com/ghewgill/picomath/blob/master/javascript/erf.js
double erf(double x) {
    // constants
    static const auto a1 = 0.254829592;
    static const auto a2 = -0.284496736;
    static const auto a3 = 1.421413741;
    static const auto a4 = -1.453152027;
    static const auto a5 = 1.061405429;
    static const auto p = 0.3275911;

    // A&S formula 7.1.26
    const auto t = 1.0 / (1.0 + p * glm::abs(x));
    const auto y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * glm::exp(-x * x);

    return static_cast<double>(glm::sign(x)) * y;
}

double defIntGaussian(double x, double mu, double sigma) {
    return 0.5 * erf((x - mu) / (1.4142135623730951 * sigma));
}

// Adapted from https://observablehq.com/@jobleonard/gaussian-kernel-calculater
std::vector<double> Engine::gaussianKernel(const size_t size, const double sigma, const double mu, const double step) {
    const auto end = 0.5 * static_cast<double>(size);
    const auto start = -end;
    std::vector<double> coeff;
    auto sum = 0.0;
    auto x = start;
    auto last_int = defIntGaussian(x, mu, sigma);
    auto acc = 0.0;
    while (x < end) {
        x += step;
        const auto new_int = defIntGaussian(x, mu, sigma);
        auto c = new_int - last_int;
        coeff.push_back(c);
        sum += c;
        last_int = new_int;
    }

    // normalize
    sum = 1 / sum;
    for (double& i : coeff) {
        i *= sum;
    }
    return coeff;
}
