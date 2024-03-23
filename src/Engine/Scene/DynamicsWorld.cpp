#include "DynamicsWorld.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static inline Vector3 toVector(const btVector3& vec) {
    return {vec.x(), vec.y(), vec.z()};
}

static inline Color4 toColor(const btVector3& color) {
    return {color.x(), color.y(), color.z(), 1.0f};
}

ContactTestObject::ContactTestObject(CollisionShape& shape) : shape{shape.clone()} {
    object = std::make_unique<btCollisionObject>();
    object->setCollisionShape(this->shape.get());
}

ContactTestObject::~ContactTestObject() = default;

btCollisionObject* ContactTestObject::get() const {
    return object.get();
}

void ContactTestObject::setTransform(const Matrix4& value) {
    btTransform transform{};
    transform.setFromOpenGLMatrix(&value[0][0]);
    object->setWorldTransform(transform);
}

class CollisionDebugDraw : public btIDebugDraw {
public:
    explicit CollisionDebugDraw(VulkanRenderer& vulkan) : vulkan{vulkan} {
    }
    ~CollisionDebugDraw() override = default;

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override {
        if (capacity < count + 1) {
            reserve();
        }

        dst = static_cast<ComponentLines::Line*>(vbo.getCurrentBuffer().getMappedPtr());

        dst[count].a = {toVector(from), toColor(color)};
        dst[count].b = {toVector(to), toColor(color)};
        ++count;
    }

    void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, const btScalar distance,
                          const int lifeTime, const btVector3& color) override {
        (void)pointOnB;
        (void)normalOnB;
        (void)distance;
        (void)lifeTime;
        (void)color;
    }

    void reportErrorWarning(const char* warningString) override {
        logger.warn("{}", warningString);
    }

    void draw3dText(const btVector3& location, const char* textString) override {
        (void)location;
        (void)textString;
    }

    void setDebugMode(const int debugMode) override {
        (void)debugMode;
    }

    int getDebugMode() const override {
        return btIDebugDraw::DBG_DrawWireframe;
    }

    const VulkanDoubleBuffer& getVbo() const {
        return vbo;
    }

    size_t getCount() const {
        return count;
    }

    void reset() {
        count = 0;
    }

private:
    void reserve() {
        capacity += 1024 * 4;
        logger.info("Expanding collision debug draw buffer to {} items", capacity);

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(ComponentLines::Line) * capacity;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vbo = vulkan.createDoubleBuffer(bufferInfo);
    }

    VulkanRenderer& vulkan;
    VulkanDoubleBuffer vbo;
    size_t capacity{0};
    size_t count{0};
    ComponentLines::Line* dst{nullptr};
};

class SimpleContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
    SimpleContactResultCallback(const CollisionMask mask) {
        m_collisionFilterMask = mask;
    }

    btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, const int partId0,
                             const int index0, const btCollisionObjectWrapper* colObj1Wrap, const int partId1,
                             const int index1) override {
        result = true;
        return 0;
    }

    [[nodiscard]] bool hasResult() const {
        return result;
    }

private:
    bool result{false};
};

DynamicsWorld::DynamicsWorld(Scene& scene, entt::registry& reg, const Config& config) :
    scene{scene},
    reg{reg},
    config{config},
    collisionConfiguration{std::make_unique<btDefaultCollisionConfiguration>()},
    dispatcher{std::make_unique<btCollisionDispatcher>(collisionConfiguration.get())},
    overlappingPairCache{std::make_unique<btDbvtBroadphase>()},
    solver{std::make_unique<btSequentialImpulseConstraintSolver>()},
    dynamicsWorld{std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), overlappingPairCache.get(), solver.get(),
                                                            collisionConfiguration.get())} {

    dynamicsWorld->setGravity(btVector3{0, 0, 0});
}

DynamicsWorld::~DynamicsWorld() = default;

void DynamicsWorld::update(const float delta) {
    dynamicsWorld->stepSimulation(delta);
}

bool DynamicsWorld::contactTest(ContactTestObject& object, const CollisionMask mask) const {
    SimpleContactResultCallback callback{mask};
    dynamicsWorld->contactTest(object.get(), callback);
    return callback.hasResult();
}

bool DynamicsWorld::contactTestSphere(const Vector3& origin, const float radius, const CollisionMask mask) const {
    btSphereShape shape{radius};
    btCollisionObject object{};

    object.setCollisionShape(&shape);
    btTransform transform{};
    transform.setIdentity();
    transform.setOrigin({origin.x, origin.y, origin.z});
    object.setWorldTransform(transform);

    SimpleContactResultCallback callback{mask};

    dynamicsWorld->contactTest(&object, callback);

    return callback.hasResult();
}

bool DynamicsWorld::contactTestBox(const Vector3& origin, float width, const CollisionMask mask) const {
    const auto half = width / 2.0f;
    btBoxShape shape{{half, half, half}};
    btCollisionObject object{};

    object.setCollisionShape(&shape);
    btTransform transform{};
    transform.setIdentity();
    transform.setOrigin({origin.x, origin.y, origin.z});
    object.setWorldTransform(transform);

    SimpleContactResultCallback callback{mask};

    dynamicsWorld->contactTest(&object, callback);

    return callback.hasResult();
}

void DynamicsWorld::updateAabbs() {
    dynamicsWorld->updateAabbs();
}

void DynamicsWorld::recalculate(VulkanRenderer& vulkan) {
    if (!debugDraw) {
        debugDraw = std::make_unique<CollisionDebugDraw>(vulkan);
        dynamicsWorld->setDebugDrawer(debugDraw.get());
    }
    if (config.graphics.debugDraw) {
        dynamic_cast<CollisionDebugDraw*>(debugDraw.get())->reset();
        dynamicsWorld->debugDrawWorld();
    }
}

const VulkanDoubleBuffer& DynamicsWorld::getDebugDrawVbo() const {
    if (!debugDraw) {
        EXCEPTION("No dynamics world debug draw setup");
    }
    return dynamic_cast<const CollisionDebugDraw*>(debugDraw.get())->getVbo();
}

size_t DynamicsWorld::getDebugDrawCount() const {
    if (!debugDraw) {
        EXCEPTION("No dynamics world debug draw setup");
    }
    return dynamic_cast<const CollisionDebugDraw*>(debugDraw.get())->getCount();
}

void DynamicsWorld::rayCast(const Vector3& start, const Vector3& end, RayCastResult& result) {
    btVector3 transFrom{start.x, start.y, start.z};
    btVector3 transTo{end.x, end.y, end.z};

    btCollisionWorld::ClosestRayResultCallback callback{transFrom, transTo};
    dynamicsWorld->rayTest(transFrom, transTo, callback);

    if (callback.hasHit()) {
        result.valid = true;
        result.hitPos = toVector(callback.m_hitPointWorld);
    } else {
        result.valid = false;
    }
}

btDynamicsWorld& DynamicsWorld::get() {
    return *dynamicsWorld;
}
