#pragma once

#include "../../assets/model.hpp"
#include "component_transform.hpp"

class btRigidBody;
class btCollisionShape;
class btMotionState;
class btDynamicsWorld;

namespace Engine {
class ENGINE_API ComponentRigidBody : public Component {
public:
    ComponentRigidBody();
    explicit ComponentRigidBody(entt::registry& reg, entt::entity handle);
    virtual ~ComponentRigidBody(); // NOLINT(modernize-use-override)
    ComponentRigidBody(const ComponentRigidBody& other) = delete;
    ComponentRigidBody(ComponentRigidBody&& other) noexcept;
    ComponentRigidBody& operator=(const ComponentRigidBody& other) = delete;
    ComponentRigidBody& operator=(ComponentRigidBody&& other) noexcept;
    static constexpr auto in_place_delete = true;

    void setFromModel(ModelPtr model, float scale);
    void setup();

    btRigidBody* getRigidBody() const {
        return rigidBody.get();
    }

    void setModel(const ModelPtr& value);
    const ModelPtr& getModel() const;

    void setLinearVelocity(const Vector3& value);
    Vector3 getLinearVelocity() const;

    void setAngularVelocity(const Vector3& value);
    Vector3 getAngularVelocity() const;

    void setWorldTransform(const Matrix4& value);
    Matrix4 getWorldTransform() const;

    void setMass(float value);
    float getMass() const;

    void setScale(float value);
    float getScale() const;

    void clearForces();
    void activate();
    bool isActive() const;

    void setDynamicsWorld(btDynamicsWorld& world) {
        dynamicsWorld = &world;
    }

    int32_t getFlags() const;
    void setFlags(int32_t value);

    static void bind(Lua& lua);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    ModelPtr model;
    float mass{1.0f};
    float scale{1.0f};
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btMotionState> motionState;
    std::unique_ptr<btRigidBody> rigidBody;
    btDynamicsWorld* dynamicsWorld{nullptr};
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::ComponentRigidBody> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::ComponentRigidBody& v) const {
            if (o.type != msgpack::type::ARRAY || o.via.array.size != 7)
                throw msgpack::type_error();
            v.setModel(o.via.array.ptr[0].as<Engine::ModelPtr>());
            v.setLinearVelocity(o.via.array.ptr[1].as<Engine::Vector3>());
            v.setAngularVelocity(o.via.array.ptr[2].as<Engine::Vector3>());
            v.setWorldTransform(o.via.array.ptr[3].as<Engine::Matrix4>());
            v.setMass(o.via.array.ptr[4].as<float>());
            v.setScale(o.via.array.ptr[5].as<float>());
            if (o.via.array.ptr[6].as<bool>()) {
                v.activate();
            }
            return o;
        }
    };

    template <> struct pack<Engine::ComponentRigidBody> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::ComponentRigidBody const& v) const {
            o.pack_array(7);
            o.pack(v.getModel());
            o.pack(v.getLinearVelocity());
            o.pack(v.getAngularVelocity());
            o.pack(v.getWorldTransform());
            o.pack(v.getMass());
            o.pack(v.getScale());
            o.pack(v.isActive());
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
