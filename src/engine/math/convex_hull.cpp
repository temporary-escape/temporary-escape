#include "convex_hull.hpp"
#include <algorithm>

// Adapted from: https://github.com/rafael-plaza/ConvexHull/blob/master/main.cpp

using namespace Engine;

bool sortPoints(const Vector2& lhs, const Vector2& rhs) {
    return (lhs.x < rhs.x) || (lhs.x == rhs.x && lhs.y < rhs.y);
}

// Check if three points make a right turn using cross product
bool right_turn(const Vector2& P1, const Vector2& P2, const Vector2& P3) {
    return ((P3.x - P1.x) * (P2.y - P1.y) - (P3.y - P1.y) * (P2.x - P1.x)) > 0;
}

std::vector<Vector2> Engine::computeConvexHull(std::vector<Vector2> points) {
    std::sort(points.begin(), points.end(), sortPoints);

    std::vector<Vector2> lowerCH;
    std::vector<Vector2> upperCH;

    // Computing upper convex hull
    upperCH.push_back(points[0]);
    upperCH.push_back(points[1]);

    for (int i = 2; i < points.size(); i++) {
        while (upperCH.size() > 1 &&
               (!right_turn(upperCH[upperCH.size() - 2], upperCH[upperCH.size() - 1], points[i]))) {
            upperCH.pop_back();
        }
        upperCH.push_back(points[i]);
    }

    // Computing lower convex hull
    lowerCH.push_back(points[points.size() - 1]);
    lowerCH.push_back(points[points.size() - 2]);

    for (int i = 2; i < points.size(); i++) {
        while (lowerCH.size() > 1 &&
               (!right_turn(lowerCH[lowerCH.size() - 2], lowerCH[lowerCH.size() - 1], points[points.size() - i - 1]))) {
            lowerCH.pop_back();
        }
        lowerCH.push_back(points[points.size() - i - 1]);
    }

    if (lowerCH.empty() || upperCH.empty()) {
        return {};
    }

    std::vector<Vector2> res;
    for (auto it = upperCH.rbegin() + 1; it != upperCH.rend(); it++) {
        res.push_back(*it);
    }
    for (auto it = lowerCH.rbegin() + 1; it != lowerCH.rend(); it++) {
        res.push_back(*it);
    }

    return res;
}
