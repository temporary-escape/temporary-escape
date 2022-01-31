#pragma once
#include "../Math/Vector.hpp"

#include <vector>

namespace Engine {
class ENGINE_API Lines {
public:
    struct Vertex {
        Vector3 pos;
        float size{0.0f};
        Color4 color;
    };

    Lines() = default;
    virtual ~Lines() = default;

    void reserve(const size_t count) {
        lines.reserve(count * 2);
    }

    void insert(const Vector3& start, const Vector3& end, const Color4& color) {
        lines.push_back({start, 1.0f, color});
        lines.push_back({end, 1.0f, color});
    }

    const std::vector<Vertex>& getLines() const {
        return lines;
    }

    bool isEmpty() const {
        return lines.empty();
    }

private:
    std::vector<Vertex> lines;
};
} // namespace Engine
