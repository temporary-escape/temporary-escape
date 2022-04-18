#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"
#include "Material.hpp"
#include "Primitive.hpp"

namespace Engine {
class ENGINE_API AssetModel : public Asset {
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

template <> struct Yaml::Adaptor<AssetModelPtr> {
    static void convert(const Yaml::Node& node, AssetModelPtr& value) {
        value = AssetModel::from(node.asString());
    }
    static void pack(Yaml::Node& node, const AssetModelPtr& value) {
        node.packString(value->getName());
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetModelPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetModelPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetModel::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetModelPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetModelPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
