#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"
#include "Material.hpp"
#include "Primitive.hpp"

namespace Scissio {
class SCISSIO_API AssetModel : public Asset {
public:
    explicit AssetModel(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetModel() = default;
    AssetModel(const AssetModel& other) = delete;
    AssetModel(AssetModel&& other) = default;

    void load(AssetManager& assetManager) override;

    const std::list<Primitive>& getPrimitives() const {
        return primitives;
    }

private:
    Path path;
    std::list<Primitive> primitives;
    Vector3 bbMin;
    Vector3 bbMax;
    float bbRadius;
};

using AssetModelPtr = std::shared_ptr<AssetModel>;

namespace Xml {
template <> void Node::convert<AssetModelPtr>(AssetModelPtr& value) const;
} // namespace Xml
} // namespace Scissio

MSGPACK_CONVERT(Scissio::AssetModelPtr)
