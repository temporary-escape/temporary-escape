#pragma once

#include "asset.hpp"
#include "image.hpp"
#include "model.hpp"

namespace Engine {
class ENGINE_API Turret : public Asset {
public:
    struct Definition {
        ModelPtr model;

        void convert(const Xml::Node& xml) {
            xml.convert("model", model);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("model", model);
        }
    };

    explicit Turret(std::string name, Path path);
    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

    [[nodiscard]] const ModelPtr& getModel() const {
        return definition.model;
    }

    static std::shared_ptr<Turret> from(const std::string& name);

    static void bind(Lua& lua);

private:
    Path path;
    Definition definition;
};

XML_DEFINE(Turret::Definition, "turret");

using TurretPtr = std::shared_ptr<Turret>;

template <> struct Xml::Adaptor<TurretPtr> {
    static void convert(const Xml::Node& node, TurretPtr& value) {
        if (node && !node.empty()) {
            value = Turret::from(std::string{node.getText()});
        }
    }
    static void pack(Xml::Node& node, const TurretPtr& value) {
        if (value) {
            node.setText(value->getName());
        }
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::TurretPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::TurretPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::Turret::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::TurretPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::TurretPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
