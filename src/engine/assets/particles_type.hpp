#pragma once

#include "../utils/aligned.hpp"
#include "asset.hpp"
#include "image.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "voxel_shape.hpp"

namespace Engine {
class ENGINE_API ParticlesType : public Asset {
public:
    struct ALIGNED(16) Uniform {
        Color4 startColor;
        Color4 endColor;
        alignas(4) float duration;
        alignas(4) int count;
        alignas(16) Vector3 direction;
        alignas(16) Vector3 startSpawn;
        alignas(16) Vector3 endSpawn;
        alignas(8) Vector2 startSize;
        alignas(8) Vector2 endSize;
    };

    struct Definition {
        struct {
            Color4 start;
            Color4 end;

            YAML_DEFINE(start, end);
        } color;

        struct {
            Vector2 start;
            Vector2 end;

            YAML_DEFINE(start, end);
        } size;

        struct {
            Vector3 start;
            Vector3 end;

            YAML_DEFINE(start, end);
        } spawn;

        Vector3 direction;
        float duration;
        int count;
        TexturePtr texture;

        YAML_DEFINE(color, size, spawn, direction, duration, count, texture);
    };

    explicit ParticlesType(std::string name, Path path);
    MOVEABLE(ParticlesType);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

    static std::shared_ptr<ParticlesType> from(const std::string& name);

    const VulkanBuffer& getUbo() const {
        return ubo;
    }

    int getCount() const {
        return definition.count;
    }

    const TexturePtr& getTexture() const {
        return definition.texture;
    }

    static void bind(Lua& lua);

private:
    Path path;
    Definition definition;
    VulkanBuffer ubo;
};

using ParticlesTypePtr = std::shared_ptr<ParticlesType>;

template <> struct Yaml::Adaptor<ParticlesTypePtr> {
    static void convert(const Yaml::Node& node, ParticlesTypePtr& value) {
        if (!node || node.isNull()) {
            value = nullptr;
        } else {
            value = ParticlesType::from(node.asString());
        }
    }
    static void pack(Yaml::Node& node, const ParticlesTypePtr& value) {
        if (value) {
            node.packString(value->getName());
        } else {
            node.packNull();
        }
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::ParticlesTypePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::ParticlesTypePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::ParticlesType::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::ParticlesTypePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::ParticlesTypePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
