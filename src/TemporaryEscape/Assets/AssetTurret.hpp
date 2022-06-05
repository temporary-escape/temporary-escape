#pragma once

#include "AssetModel.hpp"
#include "AssetParticles.hpp"

namespace Engine {
class ENGINE_API AssetTurret : public Asset {
public:
    struct Definition {
        std::string title;

        struct Component {
            AssetModelPtr model;
            Vector3 offset;

            YAML_DEFINE(model, offset);
        };

        struct Components {
            Component base;
            Component arm;
            Component cannon;

            YAML_DEFINE(base, arm, cannon);
        } components;

        YAML_DEFINE(title, components);
    };

    static std::shared_ptr<AssetTurret> from(const std::string& name);

    explicit AssetTurret(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetTurret() = default;

    void load(AssetManager& assetManager, bool noGraphics) override;

    const std::string& getTitle() const {
        return definition.title;
    }

    const Definition::Components& getComponents() const {
        return definition.components;
    }

private:
    Path path;
    Definition definition;
};

using AssetTurretPtr = std::shared_ptr<AssetTurret>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetTurretPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetTurretPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetTurret::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetTurretPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetTurretPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
