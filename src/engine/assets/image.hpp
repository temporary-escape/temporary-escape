#pragma once

#include "../utils/path.hpp"
#include "../utils/yaml.hpp"
#include "asset.hpp"
#include "image_atlas.hpp"

namespace Engine {
class ENGINE_API Image : public Asset {
public:
    explicit Image(std::string name, Path path);
    explicit Image(std::string name, const ImageAtlas::Allocation& allocation);
    void load(Registry& registry, VulkanRenderer& vulkan) override;

    [[nodiscard]] const ImageAtlas::Allocation& getAllocation() const {
        return allocation;
    }

    static std::shared_ptr<Image> from(const std::string& name);

private:
    Path path;
    ImageAtlas::Allocation allocation;
};

using ImagePtr = std::shared_ptr<Image>;

template <> struct Yaml::Adaptor<ImagePtr> {
    static void convert(const Yaml::Node& node, ImagePtr& value) {
        value = Image::from(node.asString());
    }
    static void pack(Yaml::Node& node, const ImagePtr& value) {
        node.packString(value->getName());
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::ImagePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::ImagePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::Image::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::ImagePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::ImagePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
