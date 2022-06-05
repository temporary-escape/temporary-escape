#pragma once

#include "AssetTexture.hpp"

namespace Engine {
class ENGINE_API AssetPlanet : public Asset {
public:
    struct Definition {
        std::string description;

        struct Surface {
            AssetTexturePtr texture;
            float offset{0.0f};

            YAML_DEFINE(texture, offset);
        } surface;

        YAML_DEFINE(description, surface);
    };

    static std::shared_ptr<AssetPlanet> from(const std::string& name);

    explicit AssetPlanet(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetPlanet() = default;

    void load(AssetManager& assetManager, bool noGraphics) override;

    const Texture& getTexture() const {
        return definition.surface.texture->getTexture();
    }

    const std::string& getDescription() const {
        return definition.description;
    }

    float getTextureOffset() const {
        return definition.surface.offset;
    }

private:
    Path path;
    Definition definition;
};

using AssetPlanetPtr = std::shared_ptr<AssetPlanet>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetPlanetPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetPlanetPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetPlanet::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetPlanetPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetPlanetPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
