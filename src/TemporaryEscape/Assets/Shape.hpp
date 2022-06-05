#pragma once

#include "../Graphics/Mesh.hpp"

namespace Engine {
struct Shape {
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector4 tangent;
    };

    static_assert(sizeof(Vertex) == sizeof(Vector3) * 2 + sizeof(Vector4), "struct Vertex must be tightly packed");

    enum Side : size_t {
        Default = 0,
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,
    };

    enum Type : uint8_t {
        Cube,
        Wedge,
        Corner,
        Penta,
    };

    static inline const std::array<Type, 4> allTypes = {
        Cube,
        Wedge,
        Corner,
        Penta,
    };

    static inline const std::array<std::string, 4> typeNames = {
        "shape_cube",
        "shape_wedge",
        "shape_corner",
        "shape_penta",
    };

    static inline const std::array<std::string, 4> typeFriendlyNames = {
        "Cube",
        "Wedge",
        "Corner",
        "Penta",
    };

    static inline const std::array<Side, 7> allSides = {
        Side::Default,   Side::PositiveX, Side::NegativeX, Side::PositiveY,
        Side::NegativeY, Side::PositiveZ, Side::NegativeZ,
    };

    struct Node {
        Side side = Side::Default;
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        operator bool() const {
            return !vertices.empty() && !indices.empty();
        }
    };

    std::vector<Node> sides;

    operator bool() const {
        return !sides.empty();
    }
};

inline const std::string& shapeTypeToFileName(const Shape::Type type) {
    return Shape::typeNames.at(size_t(type));
}

inline const std::string& shapeTypeToFriendlyName(const Shape::Type type) {
    return Shape::typeFriendlyNames.at(size_t(type));
}

YAML_DEFINE_ENUM(Shape::Side, PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ);
YAML_DEFINE_ENUM(Shape::Type, Cube, Wedge, Corner, Penta);
} // namespace Engine
