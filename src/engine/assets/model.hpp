#pragma once

#include "primitive.hpp"
#include "texture.hpp"

class btConvexShape;

namespace Engine {
class ENGINE_API Model : public Asset {
public:
    explicit Model(std::string name, Path path);
    Model(Model&& other) noexcept;
    Model& operator=(Model&& other) noexcept;
    ~Model() override;

    void load(AssetsManager& assetsManager, VulkanRenderer& vulkan) override;

    const std::list<Primitive>& getPrimitives() const {
        return primitives;
    }

    float getRadius() const {
        return bbRadius;
    }

    btConvexShape* getCollisionShape() const {
        return collisionShape.get();
    }

    static std::shared_ptr<Model> from(const std::string& name);

    static void bind(Lua& lua);

private:
    Path path;
    Vector3 bbMin{0.0f};
    Vector3 bbMax{0.0f};
    float bbRadius{0.0f};
    std::list<Primitive> primitives;
    std::list<Material> materials;
    std::unique_ptr<btConvexShape> collisionShape;
};

using ModelPtr = std::shared_ptr<Model>;

template <> struct Yaml::Adaptor<ModelPtr> {
    static void convert(const Yaml::Node& node, ModelPtr& value) {
        value = Model::from(node.asString());
    }
    static void pack(Yaml::Node& node, const ModelPtr& value) {
        node.packString(value->getName());
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::ModelPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::ModelPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::Model::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::ModelPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::ModelPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
