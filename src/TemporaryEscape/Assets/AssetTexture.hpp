#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Utils/Msgpack.hpp"
#include "../Utils/Yaml.hpp"
#include "Asset.hpp"

namespace Engine {
enum class TextureType {
    Unknown = 0,
    Generic,
    BaseColor,
    Normals,
    MetallicRoughness,
    AmbientOcclusion,
    Emissive,
};

class ENGINE_API AssetTexture : public Asset {
public:
    struct Options {
        struct Filtering {
            TextureFiltering minification = TextureFiltering::Linear;
            TextureFiltering magnification = TextureFiltering::Linear;

            YAML_DEFINE(minification, magnification);
        };

        struct Wrapping {
            TextureWrapping vertical = TextureWrapping::Repeat;
            TextureWrapping horizontal = TextureWrapping::Repeat;

            YAML_DEFINE(vertical, horizontal);
        };

        std::optional<bool> isArray;
        std::optional<Filtering> filtering;
        std::optional<Wrapping> wrapping;

        YAML_DEFINE(isArray, filtering, wrapping);
    };

    static std::shared_ptr<AssetTexture> from(const std::string& name);

    explicit AssetTexture(const Manifest& mod, std::string name, const Path& path, TextureType type);
    virtual ~AssetTexture() = default;

    void load(AssetManager& assetManager, bool noGraphics) override;

    const Texture& getTexture() const {
        return texture;
    }

    TextureType getType() const {
        return type;
    }

private:
    Path path;
    TextureType type;
    Texture2D texture;
    Options options{};
};

using AssetTexturePtr = std::shared_ptr<AssetTexture>;

template <> struct Yaml::Adaptor<AssetTexturePtr> {
    static void convert(const Yaml::Node& node, AssetTexturePtr& value) {
        value = AssetTexture::from(node.asString());
    }
    static void pack(Yaml::Node& node, const AssetTexturePtr& value) {
        node.packString(value->getName());
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetTexturePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetTexturePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetTexture::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetTexturePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetTexturePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
