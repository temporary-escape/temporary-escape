#pragma once
#include "../Math/Vector.hpp"

#include <vector>

namespace Scissio {
class SCISSIO_API PointCloud {
public:
    struct Vertex {
        Vector3 pos;
        float size{0.0f};
        Color4 color;
    };

    PointCloud() = default;
    virtual ~PointCloud() = default;

    void reserve(const size_t count) {
        points.reserve(count);
    }

    void insert(const Vector3& pos, const float size, const Color4& color) {
        points.push_back({pos, size, color});
    }

    const std::vector<Vertex>& getPoints() const {
        return points;
    }

    bool isEmpty() const {
        return points.empty();
    }

private:
    std::vector<Vertex> points;
};
} // namespace Scissio
