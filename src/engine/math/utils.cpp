#include "utils.hpp"

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
    const Vector4 rayClip(
        (pos.x - 0) / float(viewport.x) * 2.0f - 1.0f, -((pos.y - 0) / float(viewport.y) * 2.0f - 1.0f), 0.5f, 0.0f);

    const Vector4 rayEye = glm::inverse(projectionMatrix) * rayClip;

    const Vector4 rayEyeFixed(rayEye.x, rayEye.y, -1.0f, 0.0f);

    const auto rayWorld = viewMatrix * rayEyeFixed;
    return {rayWorld.x, rayWorld.y, rayWorld.z};
}

Vector2 Engine::worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix, const Vector2i& viewport,
                              const Vector3& pos, const bool invert) {
    const auto vp = projectionMatrix * glm::inverse(viewMatrix);
    const auto clipSpace = vp* Vector4{pos, 1.0f};
    auto ndcSpace = Vector3{clipSpace} / clipSpace.w;
    ndcSpace = Vector3{ndcSpace.x, -ndcSpace.y, ndcSpace.z};
    const auto res = ((Vector2{ndcSpace} + Vector2{1.0f}) / 2.0f) * Vector2{viewport};
    if (invert) {
        return {res.x, static_cast<float>(viewport.y) - res.y};
    }
    return res;
}

std::vector<Vector2> Engine::worldToScreen(const Matrix4& viewMatrix, const Matrix4& projectionMatrix,
                                           const Vector2i& viewport, const std::vector<Vector3>& positions,
                                           const bool invert) {
    const auto vp = projectionMatrix * glm::inverse(viewMatrix);

    std::vector<Vector2> results;
    results.resize(positions.size());

    for (size_t i = 0; i < positions.size(); i++) {
        const auto clipSpace = vp* Vector4{positions[i], 1.0f};
        auto ndcSpace = Vector3{clipSpace} / clipSpace.w;
        ndcSpace = Vector3{ndcSpace.x, -ndcSpace.y, ndcSpace.z};
        results[i] = ((Vector2{ndcSpace} + Vector2{1.0f}) / 2.0f) * Vector2{viewport};
        if (invert) {
            results[i] = Vector2{results[i].x, static_cast<float>(viewport.y) - results[i].y};
        }
    }

    return results;
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

Color4 Engine::fromRgbBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f,
    };
}

Color4 Engine::hsvToRgb(const Vector4& in) {
    float hh, p, q, t, ff;
    long i;
    Color4 out;
    out.w = in.w;

    if (in.y <= 0.0f) { // < is bogus, just shuts up warnings
        out.r = in.z;
        out.g = in.z;
        out.b = in.z;
        return out;
    }
    hh = in.r;
    if (hh >= 360.0f)
        hh = 0.0f;
    hh /= 60.0f;
    i = (long)hh;
    ff = hh - i;
    p = in.z * (1.0f - in.y);
    q = in.z * (1.0f - (in.y * ff));
    t = in.z * (1.0f - (in.y * (1.0f - ff)));

    switch (i) {
    case 0:
        out.r = in.z;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.z;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.z;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.z;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.z;
        break;
    case 5:
    default:
        out.r = in.z;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

Vector2i Engine::mipMapSize(const Vector2i& size, uint32_t level) {
    return {size.x >> level, size.y >> level};
}

Vector2i Engine::mipMapOffset(const Vector2i& size, uint32_t level) {
    if (level == 0) {
        return {0, 0};
    } else {
        Vector2i offset{size.x, 0};
        for (uint32_t i = 2; i < level + 1; i++) {
            offset.y += size.y >> (i - 1);
        }
        return offset;
    }
}
