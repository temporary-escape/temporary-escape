#pragma once

#include "asset.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "voxel_shape.hpp"

namespace Engine {
class ENGINE_API PlanetType : public Asset {
public:
    struct Definition {
        TexturePtr texture;

        YAML_DEFINE(texture);
    };

    explicit PlanetType(std::string name, Path path);
    void load(Registry& registry, VulkanRenderer& vulkan) override;

    [[nodiscard]] const TexturePtr& getTexture() const {
        return definition.texture;
    }

    static std::shared_ptr<PlanetType> from(const std::string& name);

private:
    Path path;
    Definition definition;
};

using PlanetTypePtr = std::shared_ptr<PlanetType>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::PlanetTypePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::PlanetTypePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::PlanetType::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::PlanetTypePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::PlanetTypePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
