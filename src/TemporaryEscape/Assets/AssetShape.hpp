#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"
#include "Shape.hpp"

namespace Engine {

class ENGINE_API AssetShape : public Asset {
public:
    static std::shared_ptr<AssetShape> from(const std::string& name);

    explicit AssetShape(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetShape() = default;
    AssetShape(const AssetShape& other) = delete;
    AssetShape(AssetShape&& other) = default;

    void load(AssetManager& assetManager) override;

    const Shape& getShape() const {
        return shape;
    }

private:
    Path path;
    Shape shape;
};

using AssetShapePtr = std::shared_ptr<AssetShape>;
} // namespace Engine

MSGPACK_DEFINE_ASSET(AssetShape)
