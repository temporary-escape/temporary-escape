#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Primitive.hpp"

namespace Scissio {
class SCISSIO_API Model : public Asset {
public:
    explicit Model(const Manifest& mod, std::string name, const Path& path);
    virtual ~Model() = default;
    Model(const Model& other) = delete;
    Model(Model&& other) = default;

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

using ModelPtr = std::shared_ptr<Model>;
} // namespace Scissio

MSGPACK_CONVERT(Scissio::ModelPtr)
