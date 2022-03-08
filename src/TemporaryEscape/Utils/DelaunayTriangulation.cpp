#include "DelaunayTriangulation.hpp"
#include <algorithm>
#include <optional>

using namespace Engine;

namespace std {
template <> struct hash<Vector2> {
    std::size_t operator()(const Vector2& k) const {
        const auto x = static_cast<int64_t>(k.x * 10000.0f);
        const auto y = static_cast<int64_t>(k.y * 10000.0f);
        return ((x & 0x00000000FFFFFFFF) << 32) | (y & 0x00000000FFFFFFFF);
    }
};

} // namespace std

static bool almostEqual(const float x, const float y, int ulp = 2) {
    return fabsf(x - y) <= std::numeric_limits<float>::epsilon() * fabsf(x + y) * static_cast<float>(ulp) ||
           fabsf(x - y) < std::numeric_limits<float>::min();
}

static bool almostEqual(const Vector2& v1, const Vector2& v2) {
    return almostEqual(v1.x, v2.x) && almostEqual(v1.y, v2.y);
}

static bool almostEqual(const DelaunayTriangulation::Edge& e1, const DelaunayTriangulation::Edge& e2) {
    return (almostEqual(e1.v, e2.v) && almostEqual(e1.w, e2.w)) || (almostEqual(e1.v, e2.w) && almostEqual(e1.w, e2.v));
}

static bool almostEqual(const DelaunayTriangulation::Triangle& t1, const DelaunayTriangulation::Triangle& t2) {
    return (almostEqual(t1.a, t2.a) || almostEqual(t1.a, t2.b) || almostEqual(t1.a, t2.c)) &&
           (almostEqual(t1.b, t2.a) || almostEqual(t1.b, t2.b) || almostEqual(t1.b, t2.c)) &&
           (almostEqual(t1.c, t2.a) || almostEqual(t1.c, t2.b) || almostEqual(t1.c, t2.c));
}

static float norm2(const Vector2& v) {
    return v.x * v.x + v.y * v.y;
}

static float dist2(const Vector2& a, const Vector2& b) {
    const auto dx = a.x - b.x;
    const auto dy = a.y - b.y;
    return dx * dx + dy * dy;
}

bool DelaunayTriangulation::Triangle::containsVertex(const Vector2& v) const {
    return almostEqual(a, v) || almostEqual(b, v) || almostEqual(c, v);
}

bool DelaunayTriangulation::Triangle::circumCircleContains(const Vector2& v) const {
    const auto ab = norm2(a);
    const auto cd = norm2(b);
    const auto ef = norm2(c);

    const auto ax = a.x;
    const auto ay = a.y;
    const auto bx = b.x;
    const auto by = b.y;
    const auto cx = c.x;
    const auto cy = c.y;

    const auto circum_x =
        (ab * (cy - by) + cd * (ay - cy) + ef * (by - ay)) / (ax * (cy - by) + bx * (ay - cy) + cx * (by - ay));
    const auto circum_y =
        (ab * (cx - bx) + cd * (ax - cx) + ef * (bx - ax)) / (ay * (cx - bx) + by * (ax - cx) + cy * (bx - ax));

    const Vector2 circum(circum_x / 2, circum_y / 2);
    const auto circum_radius = dist2(a, circum);
    const auto dist = dist2(v, circum);
    return dist <= circum_radius;
}

DelaunayTriangulation::DelaunayTriangulation(std::vector<Vector2> vertices) : vertices(std::move(vertices)) {
}

DelaunayTriangulation::Connections DelaunayTriangulation::solve() {
    // Determinate the super triangle
    auto minX = vertices[0].x;
    auto minY = vertices[0].y;
    auto maxX = minX;
    auto maxY = minY;

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        if (vertices[i].x < minX)
            minX = vertices[i].x;
        if (vertices[i].y < minY)
            minY = vertices[i].y;
        if (vertices[i].x > maxX)
            maxX = vertices[i].x;
        if (vertices[i].y > maxY)
            maxY = vertices[i].y;
    }

    const auto dx = maxX - minX;
    const auto dy = maxY - minY;
    const auto deltaMax = std::max(dx, dy);
    const auto midx = (minX + maxX) / 2;
    const auto midy = (minY + maxY) / 2;

    const Vector2 p1(midx - 20 * deltaMax, midy - deltaMax);
    const Vector2 p2(midx, midy + 20 * deltaMax);
    const Vector2 p3(midx + 20 * deltaMax, midy - deltaMax);

    std::vector<Triangle> triangles;

    // Create a list of triangles, and add the supertriangle in it
    triangles.push_back(Triangle(p1, p2, p3));

    for (auto p = begin(vertices); p != end(vertices); p++) {
        std::vector<Edge> polygon;

        for (auto& t : triangles) {
            if (t.circumCircleContains(*p)) {
                t.isBad = true;
                polygon.push_back(Edge{t.a, t.b});
                polygon.push_back(Edge{t.b, t.c});
                polygon.push_back(Edge{t.c, t.a});
            }
        }

        triangles.erase(std::remove_if(begin(triangles), end(triangles), [](Triangle& t) { return t.isBad; }),
                        end(triangles));

        for (auto e1 = begin(polygon); e1 != end(polygon); ++e1) {
            for (auto e2 = e1 + 1; e2 != end(polygon); ++e2) {
                if (almostEqual(*e1, *e2)) {
                    e1->isBad = true;
                    e2->isBad = true;
                }
            }
        }

        polygon.erase(std::remove_if(begin(polygon), end(polygon), [](Edge& e) { return e.isBad; }), end(polygon));

        for (const auto& e : polygon)
            triangles.push_back(Triangle(e.v, e.w, *p));
    }

    triangles.erase(std::remove_if(begin(triangles), end(triangles),
                                   [p1, p2, p3](Triangle& t) {
                                       return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
                                   }),
                    end(triangles));

    std::unordered_map<Vector2, size_t> vertexToIndex;
    vertexToIndex.reserve(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) {
        vertexToIndex.insert(std::make_pair(vertices.at(i), i));
    }

    Connections connections;
    connections.reserve(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) {
        connections.insert(std::make_pair(i, std::vector<size_t>{}));
    }

    const auto getIndex = [&](const Vector2& v) -> std::optional<size_t> {
        const auto it = vertexToIndex.find(v);
        if (it == vertexToIndex.end()) {
            return std::nullopt;
        }
        return it->second;
    };

    const auto addConnection = [&](const Vector2& a, const Vector2& b) {
        const auto idxA = getIndex(a);
        const auto idxB = getIndex(b);

        if (!idxA || !idxB) {
            return;
        }

        auto it = connections.find(idxA.value());

        if (std::find(it->second.begin(), it->second.end(), idxB.value()) == it->second.end()) {
            it->second.push_back(idxB.value());
        }
    };

    for (const auto& t : triangles) {
        addConnection(t.a, t.b);
        addConnection(t.b, t.c);
        addConnection(t.c, t.a);
    }

    return connections;

    /*for (const auto t : triangles) {
        edges.push_back(Edge{t.a, t.b});
        edges.push_back(Edge{t.b, t.c});
        edges.push_back(Edge{t.c, t.a});
    }*/
}
