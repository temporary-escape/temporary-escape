#pragma once

#include "../Utils/Path.hpp"
#include "../Utils/Xml.hpp"
#include "../Vulkan/VulkanTexture.hpp"
#include "Asset.hpp"

namespace Engine {
class ENGINE_API Texture : public Asset {
public:
    enum class Type {
        Texture2D,
        Texture1D,
    };

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
            Filtering minification{Filtering::Linear};
            Filtering magnification{Filtering::Linear};

            void convert(const Xml::Node& xml) {
                xml.convert("minification", minification);
                xml.convert("magnification", magnification);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("minification", minification);
                xml.pack("magnification", magnification);
            }
        };

        struct OptionsWrapping {
            Wrapping vertical{Wrapping::Repeat};
            Wrapping horizontal{Wrapping::Repeat};

            void convert(const Xml::Node& xml) {
                xml.convert("vertical", vertical);
                xml.convert("horizontal", horizontal);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("vertical", vertical);
                xml.pack("horizontal", horizontal);
            }
        };

        bool isArray{false};
        bool compress{true};
        bool srgb{false};
        Type type{Type::Texture2D};
        OptionsFiltering filtering{};
        OptionsWrapping wrapping{};

        void convert(const Xml::Node& xml) {
            xml.convert("isArray", isArray, false);
            xml.convert("compress", compress, false);
            xml.convert("srgb", srgb, false);
            xml.convert("type", type, false);
            xml.convert("filtering", filtering, false);
            xml.convert("wrapping", wrapping, false);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("isArray", isArray);
            xml.pack("compress", compress);
            xml.pack("srgb", srgb);
            xml.pack("type", type);
            xml.pack("filtering", filtering);
            xml.pack("wrapping", wrapping);
        }

        void apply(VulkanTexture::CreateInfo& textureInfo) const;
    };

    static Options loadOptions(const Path& path);

    explicit Texture(std::string name, Path path);
    MOVEABLE(Texture);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

    VulkanTexture& getVulkanTexture() {
        return texture;
    }

    [[nodiscard]] const VulkanTexture& getVulkanTexture() const {
        return texture;
    }

    [[nodiscard]] const Path& getPath() const {
        return path;
    }

    static std::shared_ptr<Texture> from(const std::string& name);

    static void bind(Lua& lua);

private:
    void loadPng(const Options& options, VulkanRenderer& vulkan);
    void loadKtx2(const Options& options, VulkanRenderer& vulkan);

    Path path;
    VulkanTexture texture;
};

XML_DEFINE(Texture::Options, "texture");

using TexturePtr = std::shared_ptr<Texture>;

template <> struct Xml::Adaptor<TexturePtr> {
    static void convert(const Xml::Node& node, TexturePtr& value) {
        value = Texture::from(std::string{node.getText()});
    }
    static void pack(Xml::Node& node, const TexturePtr& value) {
        node.setText(value->getName());
    }
};

XML_DEFINE_ENUM(Texture::Wrapping, ClampToEdge, Repeat);
XML_DEFINE_ENUM(Texture::Filtering, Linear, Nearest);
XML_DEFINE_ENUM(Texture::Type, Texture1D, Texture2D);
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
