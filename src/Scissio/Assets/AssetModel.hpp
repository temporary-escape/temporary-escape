#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"
#include "Material.hpp"
#include "Primitive.hpp"

namespace Scissio {
class SCISSIO_API AssetModel : public Asset {
public:
    static std::shared_ptr<AssetModel> from(const std::string& name);

    explicit AssetModel(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetModel() = default;
    AssetModel(const AssetModel& other) = delete;
    AssetModel(AssetModel&& other) = default;

    void load(AssetManager& assetManager) override;

    const std::list<Primitive>& getPrimitives() const {
        return primitives;
    }

    float getBoundingRadius() const {
        return bbRadius;
    }

private:
    Path path;
    std::list<Primitive> primitives;
    Vector3 bbMin;
    Vector3 bbMax;
    float bbRadius;
};

using AssetModelPtr = std::shared_ptr<AssetModel>;

struct WeightedModel {
    AssetModelPtr model;
    float weight{1.0f};
};

namespace Xml {
template <> struct Adaptor<WeightedModel> { static void convert(const Xml::Node& n, WeightedModel& v); };
template <> struct Adaptor<AssetModelPtr> { static void convert(const Xml::Node& n, AssetModelPtr& v); };
} // namespace Xml
} // namespace Scissio

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Scissio::AssetModelPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::AssetModelPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Scissio::AssetModel::from(name);
            return o;
        }
    };

    template <> struct pack<Scissio::AssetModelPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::AssetModelPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
