#pragma once

#include "../../Assets/Model.hpp"
#include "ComponentTransform.hpp"

class btRigidBody;
class btCollisionShape;
class btMotionState;
class btDynamicsWorld;

namespace Engine {
class ENGINE_API ComponentRigidBody : public Component {
public:
    ComponentRigidBody();
    explicit ComponentRigidBody(EntityId entity);
    virtual ~ComponentRigidBody(); // NOLINT(modernize-use-override)
    ComponentRigidBody(const ComponentRigidBody& other) = delete;
    ComponentRigidBody(ComponentRigidBody&& other) noexcept;
    ComponentRigidBody& operator=(const ComponentRigidBody& other) = delete;
    ComponentRigidBody& operator=(ComponentRigidBody&& other) noexcept;
    static constexpr auto in_place_delete = true;

    [[nodiscard]] btRigidBody* getRigidBody() const {
        return rigidBody.get();
    }

    void setShape(ComponentTransform& transform, const CollisionShape& shape);
    void setShape(ComponentTransform& transform, std::unique_ptr<btCollisionShape> value);

    void setLinearVelocity(const Vector3& value);
    [[nodiscard]] Vector3 getLinearVelocity() const;

    void setAngularVelocity(const Vector3& value);
    [[nodiscard]] Vector3 getAngularVelocity() const;

    void setWorldTransform(const Matrix4& value);
    [[nodiscard]] Matrix4 getWorldTransform() const;

    void setBtTransform(const btTransform& value);
    const btTransform& getBtTransform() const;

    void setMass(const float value) {
        mass = value;
    }
    [[nodiscard]] float getMass() const {
        return mass;
    }

    void setScale(const float value) {
        scale = value;
    }
    [[nodiscard]] float getScale() const {
        return scale;
    }

    // void clearForces();
    void activate();
    [[nodiscard]] bool isActive() const;

    // void resetTransform(const Vector3& pos, const Quaternion& rot);
    // void updateTransform();

    void setDynamicsWorld(btDynamicsWorld& world) {
        dynamicsWorld = &world;
    }

    /*void setKinematic(const bool value) {
        kinematic = value;
    }
    [[nodiscard]] bool getKinematic() const {
        return kinematic;
    }*/

    [[nodiscard]] int32_t getFlags() const;
    void setFlags(int32_t value);

private:
    float mass{1.0f};
    float scale{1.0f};
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btRigidBody> rigidBody;
    btMotionState* motionState{nullptr};
    btDynamicsWorld* dynamicsWorld{nullptr};
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<btTransform> {
        msgpack::object const& operator()(msgpack::object const& o, btTransform& v) const {
            if (o.type != msgpack::type::BIN || o.via.bin.size != sizeof(btTransform))
                throw msgpack::type_error();
            v = *reinterpret_cast<const btTransform*>(o.via.bin.ptr);
            return o;
        }
    };

    template <> struct pack<btTransform> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, btTransform const& v) const {
            o.pack_bin(sizeof(btTransform));
            o.pack_bin_body(reinterpret_cast<const char*>(&v), sizeof(btTransform));
            return o;
        }
    };

    template <> struct convert<Engine::ComponentRigidBody> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::ComponentRigidBody& v) const {
            if (o.type != msgpack::type::ARRAY || o.via.array.size != 6)
                throw msgpack::type_error();
            v.setLinearVelocity(o.via.array.ptr[0].as<Engine::Vector3>());
            v.setAngularVelocity(o.via.array.ptr[1].as<Engine::Vector3>());
            // v.setKinematic(o.via.array.ptr[2].as<bool>());
            /*if (!v.getKinematic()) {
                v.setWorldTransform(o.via.array.ptr[3].as<Engine::Matrix4>());
            }*/
            if (v.getRigidBody()) {
                v.setBtTransform(o.via.array.ptr[2].as<btTransform>());
            }
            v.setMass(o.via.array.ptr[3].as<float>());
            v.setScale(o.via.array.ptr[4].as<float>());
            if (o.via.array.ptr[5].as<bool>()) {
                v.activate();
            }
            return o;
        }
    };

    template <> struct pack<Engine::ComponentRigidBody> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::ComponentRigidBody const& v) const {
            o.pack_array(6);
            o.pack(v.getLinearVelocity());
            o.pack(v.getAngularVelocity());
            if (v.getRigidBody()) {
                o.pack(v.getBtTransform());
            } else {
                o.pack_nil();
            }
            o.pack(v.getMass());
            o.pack(v.getScale());
            o.pack(v.isActive());
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
