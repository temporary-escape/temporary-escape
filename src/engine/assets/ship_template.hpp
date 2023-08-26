#pragma once

#include "../graphics/planet_textures.hpp"
#include "asset.hpp"
#include "image.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "voxel_shape.hpp"

namespace Engine {
class ENGINE_API ShipTemplate : public Asset {
public:
    explicit ShipTemplate(std::string name, Path path);
    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

    static std::shared_ptr<ShipTemplate> from(const std::string& name);

    const Path& getPath() const {
        return path;
    }

    static void bind(Lua& lua);

private:
    Path path;
};

using ShipTemplatePtr = std::shared_ptr<ShipTemplate>;

template <> struct Xml::Adaptor<ShipTemplatePtr> {
    static void convert(const Xml::Node& node, ShipTemplatePtr& value) {
        if (node && !node.empty()) {
            value = ShipTemplate::from(std::string{node.getText()});
        }
    }
    static void pack(Xml::Node& node, const ShipTemplatePtr& value) {
        if (value) {
            node.setText(value->getName());
        }
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::ShipTemplatePtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::ShipTemplatePtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::ShipTemplate::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::ShipTemplatePtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::ShipTemplatePtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
