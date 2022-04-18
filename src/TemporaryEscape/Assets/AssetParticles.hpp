#pragma once

#include "AssetTexture.hpp"

namespace Engine {
class ENGINE_API AssetParticles : public Asset {
public:
    struct Definition {
        AssetTexturePtr texture;
        Color4 startColor{1.0f};
        Color4 endColor{1.0f};
        Vector3 force{0.0f};
        float startRadius{1.0f};
        float endRadius{1.0f};
        int count{100};
        float duration{1.0f};
        float startSize{1.0f};
        float endSize{1.0f};

        YAML_DEFINE(texture, startColor, endColor, force, startRadius, endRadius, count, duration, startSize, endSize);
    };

    static std::shared_ptr<AssetParticles> from(const std::string& name);

    explicit AssetParticles(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetParticles() = default;

    void load(AssetManager& assetManager) override;

    const AssetTexturePtr& getTexture() const {
        return definition.texture;
    }

    const Color4& getStartColor() const {
        return definition.startColor;
    }

    const Color4& getEndColor() const {
        return definition.endColor;
    }

    const Vector3& getForce() const {
        return definition.force;
    }

    float getStartRadius() const {
        return definition.startRadius;
    }

    float getEndRadius() const {
        return definition.endRadius;
    }

    int getCount() const {
        return definition.count;
    }

    float getDuration() const {
        return definition.duration;
    }

    float getStartSize() const {
        return definition.startSize;
    }

    float getEndSize() const {
        return definition.endSize;
    }

private:
    Path path;
    Definition definition;
};

using AssetParticlesPtr = std::shared_ptr<AssetParticles>;

template <> struct Yaml::Adaptor<AssetParticlesPtr> {
    static void convert(const Yaml::Node& node, AssetParticlesPtr& value) {
        value = AssetParticles::from(node.asString());
    }
    static void pack(Yaml::Node& node, const AssetParticlesPtr& value) {
        node.packString(value->getName());
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetParticlesPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetParticlesPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetParticles::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetParticlesPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetParticlesPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
