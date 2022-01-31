#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Utils/Msgpack.hpp"
#include "Asset.hpp"

namespace Engine {
class ENGINE_API AssetImage : public Asset {
public:
    static std::shared_ptr<AssetImage> from(const std::string& name);

    explicit AssetImage(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetImage() = default;

    void load(AssetManager& assetManager) override;

    [[nodiscard]] const Canvas2D::Image& getImage() const {
        return image;
    }

private:
    Path path;
    Canvas2D::Image image;
};

using AssetImagePtr = std::shared_ptr<AssetImage>;

namespace Xml {
template <> struct Adaptor<AssetImagePtr> { static void convert(const Xml::Node& n, AssetImagePtr& v); };
} // namespace Xml
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetImagePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetImagePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetImage::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetImagePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetImagePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
