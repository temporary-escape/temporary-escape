#pragma once

#include "../utils/path.hpp"
#include "../utils/yaml.hpp"
#include "../vulkan/vulkan_texture.hpp"
#include "asset.hpp"

namespace Engine {
class ENGINE_API Texture : public Asset {
public:
    enum class Wrapping {
        ClampToEdge,
        Repeat,
    };

    enum class Filtering {
        Linear,
        Nearest,
    };

    struct Options {
        struct OptionsFiltering {
            Filtering minification = Filtering::Linear;
            Filtering magnification = Filtering::Linear;

            YAML_DEFINE(minification, magnification);
        };

        struct OptionsWrapping {
            Wrapping vertical = Wrapping::Repeat;
            Wrapping horizontal = Wrapping::Repeat;

            YAML_DEFINE(vertical, horizontal);
        };

        std::optional<bool> isArray;
        std::optional<OptionsFiltering> filtering;
        std::optional<OptionsWrapping> wrapping;

        YAML_DEFINE(isArray, filtering, wrapping);

        void apply(VulkanTexture::CreateInfo& textureInfo);
    };

    explicit Texture(std::string name, Path path);
    void load(Registry& registry, VulkanRenderer& vulkan) override;

    VulkanTexture& getVulkanTexture() {
        return texture;
    }

    [[nodiscard]] const VulkanTexture& getVulkanTexture() const {
        return texture;
    }

    static std::shared_ptr<Texture> from(const std::string& name);

private:
    Path path;
    VulkanTexture texture;
};

using TexturePtr = std::shared_ptr<Texture>;

template <> struct Yaml::Adaptor<TexturePtr> {
    static void convert(const Yaml::Node& node, TexturePtr& value) {
        value = Texture::from(node.asString());
    }
    static void pack(Yaml::Node& node, const TexturePtr& value) {
        node.packString(value->getName());
    }
};

YAML_DEFINE_ENUM(Texture::Wrapping, ClampToEdge, Repeat);

YAML_DEFINE_ENUM(Texture::Filtering, Linear, Nearest);
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::TexturePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::TexturePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::Texture::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::TexturePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::TexturePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
