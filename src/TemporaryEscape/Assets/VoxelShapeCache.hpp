#pragma once

#include "../Config.hpp"
#include "../Math/Matrix.hpp"
#include "VoxelShape.hpp"

namespace Engine {
class ENGINE_API VoxelShapeCache {
public:
    explicit VoxelShapeCache(const Config& config);

    struct ShapePrebuilt {
        std::vector<VoxelShape::VertexCached> vertices;
        std::vector<uint16_t> indices;
    };

    using ShapePrebuiltMasked = std::array<ShapePrebuilt, 64>;
    using ShapePrebuiltRotated = std::array<ShapePrebuiltMasked, 24>;
    using ShapesPrebuilt = std::array<ShapePrebuiltRotated, 4>;

    void init();
    void precompute();

    static const Matrix4& getRotationMatrix(const uint8_t rotation);
    static const Matrix4& getRotationMatrixInverted(const uint8_t rotation);

    const ShapesPrebuilt& getShapes() const {
        return shapesPrebuilt;
    }

private:
    const Config& config;
    std::unordered_map<VoxelShape::Type, VoxelShape> shapes;
    ShapesPrebuilt shapesPrebuilt;
};
} // namespace Engine
