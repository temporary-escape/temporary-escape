#pragma once

#include "../Library.hpp"
#include "Vector.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API DelaunayTriangulation {
public:
    class Edge {
    public:
        Edge() = default;
        explicit Edge(const Vector2& v, const Vector2& w) : v(v), w(w) {
        }

        Vector2 v{};
        Vector2 w{};
        bool isBad{false};

        bool operator==(const Edge& other) const {
            return v == other.v && w == other.w;
        }
    };

    class Triangle {
    public:
        Triangle() = default;
        explicit Triangle(const Vector2& a, const Vector2& b, const Vector2& c) : a(a), b(b), c(c) {
        }

        Vector2 a{};
        Vector2 b{};
        Vector2 c{};
        bool isBad{false};

        [[nodiscard]] bool containsVertex(const Vector2& v) const;
        [[nodiscard]] bool circumCircleContains(const Vector2& v) const;

        bool operator==(const Triangle& other) const {
            return a == other.a && b == other.b && c == other.c;
        }
    };

    using Connections = std::unordered_map<size_t, std::vector<size_t>>;

    explicit DelaunayTriangulation(const std::vector<Vector2>& points);
    Connections solve();

private:
    const std::vector<Vector2>& vertices;
};
} // namespace Engine
