#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"

namespace Scissio {
class SCISSIO_API AssetTexture : public Asset {
public:
    struct Options {
        struct Filtering {
            TextureFiltering minification = TextureFiltering::Linear;
            TextureFiltering magnification = TextureFiltering::Linear;
        } filtering;

        struct Wrapping {
            TextureWrapping vertical = TextureWrapping::Repeat;
            TextureWrapping horizontal = TextureWrapping::Repeat;
        } wrapping;
    };

    static std::shared_ptr<AssetTexture> from(const std::string& name);

    explicit AssetTexture(const Manifest& mod, std::string name, const Path& path, TextureType type);
    virtual ~AssetTexture() = default;

    void load(AssetManager& assetManager) override;

    const Texture2D& getTexture() const {
        return texture;
    }

    TextureType getType() const {
        return type;
    }

private:
    Path path;
    TextureType type;
    Texture2D texture;
};

using AssetTexturePtr = std::shared_ptr<AssetTexture>;

namespace Xml {
template <> struct Adaptor<AssetTexture::Options::Filtering> {
    static void convert(const Xml::Node& n, AssetTexture::Options::Filtering& v);
};
template <> struct Adaptor<AssetTexture::Options::Wrapping> {
    static void convert(const Xml::Node& n, AssetTexture::Options::Wrapping& v);
};
template <> struct Adaptor<AssetTexture::Options> {
    static void convert(const Xml::Node& n, AssetTexture::Options& v);
};
template <> struct Adaptor<AssetTexturePtr> { static void convert(const Xml::Node& n, AssetTexturePtr& v); };
} // namespace Xml
} // namespace Scissio

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Scissio::AssetTexturePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::AssetTexturePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Scissio::AssetTexture::from(name);
            return o;
        }
    };

    template <> struct pack<Scissio::AssetTexturePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::AssetTexturePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
