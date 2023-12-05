#pragma once

#include "../Utils/Aligned.hpp"
#include "Asset.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "VoxelShape.hpp"

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

            void convert(const Xml::Node& xml) {
                xml.convert("start", start);
                xml.convert("end", end);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("start", start);
                xml.pack("end", end);
            }
        } color;

        struct {
            Vector2 start;
            Vector2 end;

            void convert(const Xml::Node& xml) {
                xml.convert("start", start);
                xml.convert("end", end);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("start", start);
                xml.pack("end", end);
            }
        } size;

        struct {
            Vector3 start;
            Vector3 end;

            void convert(const Xml::Node& xml) {
                xml.convert("start", start);
                xml.convert("end", end);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("start", start);
                xml.pack("end", end);
            }
        } spawn;

        Vector3 direction;
        float duration;
        int count;
        TexturePtr texture;

        void convert(const Xml::Node& xml) {
            xml.convert("color", color);
            xml.convert("size", size);
            xml.convert("spawn", spawn);
            xml.convert("direction", direction);
            xml.convert("duration", duration);
            xml.convert("count", count);
            xml.convert("texture", texture);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("color", color);
            xml.pack("size", size);
            xml.pack("spawn", spawn);
            xml.pack("direction", direction);
            xml.pack("duration", duration);
            xml.pack("count", count);
            xml.pack("texture", texture);
        }
    };

    explicit ParticlesType(std::string name, Path path);
    MOVEABLE(ParticlesType);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

    static std::shared_ptr<ParticlesType> from(const std::string& name);

    [[nodiscard]] const VulkanBuffer& getUbo() const {
        return ubo;
    }

    [[nodiscard]] int getCount() const {
        return definition.count;
    }

    [[nodiscard]] const TexturePtr& getTexture() const {
        return definition.texture;
    }

private:
    Path path;
    Definition definition;
    VulkanBuffer ubo;
};

XML_DEFINE(ParticlesType::Definition, "particles");

using ParticlesTypePtr = std::shared_ptr<ParticlesType>;

template <> struct Xml::Adaptor<ParticlesTypePtr> {
    static void convert(const Xml::Node& node, ParticlesTypePtr& value) {
        if (node && !node.empty()) {
            value = ParticlesType::from(std::string{node.getText()});
        }
    }
    static void pack(Xml::Node& node, const ParticlesTypePtr& value) {
        if (value) {
            node.setText(value->getName());
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
