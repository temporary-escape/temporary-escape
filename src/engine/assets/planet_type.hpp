#pragma once

#include "../graphics/planet_textures.hpp"
#include "asset.hpp"
#include "image.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "voxel_shape.hpp"

namespace Engine {
class ENGINE_API PlanetType : public Asset {
public:
    struct Definition {
        TexturePtr biome;
        TexturePtr roughness;

        struct Amotsphere {
            Color4 start;
            Color4 end;
            float strength{0.0f};
            float waterLevel{0.0f};

            YAML_DEFINE(start, end, strength, waterLevel);
        } atmosphere;

        YAML_DEFINE(biome, roughness, atmosphere);
    };

    explicit PlanetType(std::string name, Path path);
    void load(AssetsManager& assetsManager, VulkanRenderer& vulkan, AudioContext& audio) override;

    [[nodiscard]] const TexturePtr& getBiomeTexture() const {
        return definition.biome;
    }

    [[nodiscard]] const TexturePtr& getRoughnessTexture() const {
        return definition.roughness;
    }

    [[nodiscard]] const Definition::Amotsphere& getAtmosphere() const {
        return definition.atmosphere;
    }

    [[nodiscard]] const VulkanBuffer& getUbo() const {
        return ubo;
    }

    void setLowResTextures(PlanetTextures value) {
        textures = std::move(value);
    }

    [[nodiscard]] const PlanetTextures& getLowResTextures() const {
        return textures;
    }

    void setThumbnail(ImagePtr value) {
        thumbnail = std::move(value);
    }

    [[nodiscard]] const ImagePtr& getThumbnail() const {
        return thumbnail;
    }

    static std::shared_ptr<PlanetType> from(const std::string& name);

    static void bind(Lua& lua);

private:
    Path path;
    Definition definition;
    VulkanBuffer ubo;
    PlanetTextures textures;
    ImagePtr thumbnail;
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
