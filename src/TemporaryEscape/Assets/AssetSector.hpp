#pragma once

#include "AssetModel.hpp"
#include "AssetParticles.hpp"

namespace Engine {
class ENGINE_API AssetSector : public Asset {
public:
    static std::shared_ptr<AssetSector> from(const std::string& name);

    explicit AssetSector(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetSector() = default;

    void load(AssetManager& assetManager) override;

private:
    Path path;
};

using AssetSectorPtr = std::shared_ptr<AssetSector>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetSectorPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetSectorPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetSector::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetSectorPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetSectorPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
