#pragma once

#include "AssetModel.hpp"

namespace Scissio {
class SCISSIO_API AssetAsteroid : public Asset {
public:
    struct Definition {
        std::string name;
        std::vector<WeightedModel> models;
    };

    static std::shared_ptr<AssetAsteroid> from(const std::string& name);

    explicit AssetAsteroid(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetAsteroid() = default;

    void load(AssetManager& assetManager) override;

    const std::string& getName() const {
        return definition.name;
    }

    const std::vector<WeightedModel>& getModels() const {
        return definition.models;
    }

private:
    Path path;
    Definition definition;
};

using AssetAsteroidPtr = std::shared_ptr<AssetAsteroid>;

namespace Xml {
template <> struct Adaptor<AssetAsteroid::Definition> {
    static void convert(const Xml::Node& n, AssetAsteroid::Definition& v);
};
} // namespace Xml
} // namespace Scissio

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Scissio::AssetAsteroidPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::AssetAsteroidPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Scissio::AssetAsteroid::from(name);
            return o;
        }
    };

    template <> struct pack<Scissio::AssetAsteroidPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::AssetAsteroidPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
