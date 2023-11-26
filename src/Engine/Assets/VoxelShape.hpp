#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Xml.hpp"
#include "../Vulkan/VulkanTypes.hpp"
#include <string>
#include <vector>

namespace Engine {
struct ENGINE_API VoxelShape {
    struct VertexCached {
        Vector3 position;
        Vector3 normal;
    };

    static constexpr size_t numOfShapes = 4;

    struct VertexFinal {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector4 tangent;
        float color;
        float padding[3];

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexFinal, position)},
                {1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexFinal, normal)},
                {2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexFinal, texCoords)},
                {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexFinal, tangent)},
                {4, VK_FORMAT_R32_SFLOAT, offsetof(VertexFinal, color)},
            };
        };
    };

    static_assert(sizeof(VertexCached) == sizeof(float) * 6, "struct Vertex must be tightly packed");
    static_assert(sizeof(VertexFinal) == sizeof(float) * 16, "struct VertexFinal must be tightly packed");

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

    static const std::array<Type, numOfShapes> allTypes;
    static const std::array<std::string, numOfShapes> typeNames;
    static const std::array<std::string, numOfShapes> typeFriendlyNames;
    static const std::array<Face, 7> allSides;

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

XML_DEFINE_ENUM(VoxelShape::Face, PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ);
XML_DEFINE_ENUM(VoxelShape::Type, Cube, Wedge, Corner, Penta);
} // namespace Engine
