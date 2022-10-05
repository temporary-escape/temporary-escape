#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Yaml.hpp"
#include <string>
#include <vector>

namespace Engine {
struct ENGINE_API VoxelShape {
    struct VertexCached {
        Vector3 position;
        Vector3 normal;
    };

    struct VertexFinal {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector4 tangent;
    };

    static_assert(sizeof(VertexCached) == sizeof(float) * 6, "struct Vertex must be tightly packed");
    static_assert(sizeof(VertexFinal) == sizeof(float) * 12, "struct VertexFinal must be tightly packed");

    enum Face : size_t {
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

    static inline const std::array<Face, 7> allSides = {
        Face::Default,   Face::PositiveX, Face::NegativeX, Face::PositiveY,
        Face::NegativeY, Face::PositiveZ, Face::NegativeZ,
    };

    struct Node {
        Face side = Face::Default;
        std::vector<VertexCached> vertices;
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

inline const std::string& shapeTypeToFileName(const VoxelShape::Type type) {
    return VoxelShape::typeNames.at(size_t(type));
}

inline const std::string& shapeTypeToFriendlyName(const VoxelShape::Type type) {
    return VoxelShape::typeFriendlyNames.at(size_t(type));
}

YAML_DEFINE_ENUM(VoxelShape::Face, PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ);
YAML_DEFINE_ENUM(VoxelShape::Type, Cube, Wedge, Corner, Penta);
} // namespace Engine
