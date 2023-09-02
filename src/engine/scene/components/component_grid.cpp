#include "component_grid.hpp"
#include "../../file/msgpack_file_reader.hpp"
#include "../../file/teb_file_header.hpp"
#include "../../server/lua.hpp"
#include <btBulletDynamicsCommon.h>
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static auto boxShape = std::make_unique<btBoxShape>(btVector3{0.5f, 0.5f, 0.5f});

class ComponentGridMotionState : public btMotionState {
public:
    explicit ComponentGridMotionState(ComponentGrid& componentGrid, ComponentTransform& componentTransform) :
        componentGrid{componentGrid}, componentTransform{componentTransform} {
    }

    ~ComponentGridMotionState() override = default;

    void getWorldTransform(btTransform& worldTrans) const override {
        const auto mat = componentTransform.getAbsoluteTransform();
        worldTrans.setFromOpenGLMatrix(&mat[0][0]);
    }

    void setWorldTransform(const btTransform& worldTrans) override {
        Matrix4 mat;
        worldTrans.getOpenGLMatrix(&mat[0][0]);
        componentTransform.setTransform(mat);
        componentGrid.setDirty(true);
    }

private:
    ComponentGrid& componentGrid;
    ComponentTransform& componentTransform;
};

ComponentGrid::ComponentGrid() = default;

ComponentGrid::ComponentGrid(entt::registry& reg, entt::entity handle) : Component{reg, handle} {
}

ComponentGrid::~ComponentGrid() {
    clear();
}

ComponentGrid::ComponentGrid(ComponentGrid&& other) noexcept = default;

ComponentGrid& ComponentGrid::operator=(ComponentGrid&& other) noexcept = default;

void ComponentGrid::clear() {
    for (auto& primitive : primitives) {
        if (vulkanRenderer) {
            vulkanRenderer->dispose(std::move(primitive.mesh.ibo));
            vulkanRenderer->dispose(std::move(primitive.mesh.vbo));
        }
    }
    primitives.clear();
}

void ComponentGrid::update() {
}

void ComponentGrid::createShape(btCompoundShape& compoundShape, Grid::Iterator iterator) {
    while (iterator) {
        if (iterator.isVoxel()) {
            auto pos = iterator.getPos();
            btTransform transform{};
            transform.setIdentity();
            transform.setOrigin({pos.x, pos.y, pos.z});
            compoundShape.addChildShape(transform, boxShape.get());
        } else {
            createShape(compoundShape, iterator.children());
        }

        iterator.next();
    }
}

void ComponentGrid::updateShape(btDynamicsWorld& dynamicsWorld) {
    if (rigidBody) {
        return;
    }

    logger.info("ComponentGrid::updateShape");

    btVector3 localInertia{0.0f, 0.0f, 0.0f};
    btScalar mass{1.0f};

    shape = std::make_unique<btCompoundShape>();
    auto& compoundShape = *static_cast<btCompoundShape*>(shape.get());

    auto iterator = iterate();
    createShape(compoundShape, iterator);

    compoundShape.recalculateLocalAabb();

    shape->calculateLocalInertia(mass, localInertia);

    if (!rigidBody) {
        auto transform = tryGet<ComponentTransform>();
        if (!transform) {
            EXCEPTION("ComponentGrid added on entity with no ComponentTransform");
        }
        motionState = std::unique_ptr<btMotionState>{new ComponentGridMotionState(*this, *transform)};

        logger.info("ComponentGrid::updateShape new rigidBody");
        btRigidBody::btRigidBodyConstructionInfo rbInfo{mass, motionState.get(), shape.get(), localInertia};
        rigidBody = std::make_unique<btRigidBody>(rbInfo);
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
        btTransform trans;
        motionState->getWorldTransform(trans);
        rigidBody->setWorldTransform(trans);

        dynamicsWorld.addRigidBody(rigidBody.get());
    } else {
        logger.info("ComponentGrid::updateShape setCollisionShape");
        rigidBody->setCollisionShape(shape.get());
    }
}

void ComponentGrid::setFrom(const ShipTemplatePtr& shipTemplate) {
    if (!shipTemplate) {
        EXCEPTION("Can not set grid from null ship template");
    }

    clear();

    MsgpackFileReader file{shipTemplate->getPath()};
    TebFileHeader header{};
    file.unpack(header);

    if (header.type != TebFileType::Ship) {
        EXCEPTION("Can not set grid from ship template, bad file type");
    }

    file.unpack(*this);
    setDirty(true);
}

void ComponentGrid::debugIterate(Grid::Iterator iterator) {
    while (iterator) {
        if (iterator.isVoxel()) {
            auto pos = iterator.getPos();
            // logger.debug("Add box: {}", pos);
            debug->addBox(glm::translate(pos), 0.95f, Color4{1.0f, 0.0f, 0.0f, 1.0f});
        } else {
            debugIterate(iterator.children());
        }

        iterator.next();
    }
}

void ComponentGrid::createParticlesVertices(Grid::Iterator iterator) {
    while (iterator) {
        if (iterator.isVoxel()) {
            auto pos = iterator.getPos();
            auto& cache = blockCache.at(iterator.value().voxel.type.value());
            if (cache.particles.type) {
                const auto test = find(pos + Vector3i{0, 0, 1});
                if (!test) {
                    auto& mat = particles[cache.particles.type].emplace_back();
                    mat = glm::translate(Matrix4{1.0f}, Vector3{pos} + cache.particles.offset);
                }
            }
        } else {
            createParticlesVertices(iterator.children());
        }

        iterator.next();
    }
}

void ComponentGrid::recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache) {
    vulkanRenderer = &vulkan;

    blockCache.clear();
    blockCache.resize(Grid::getTypeCount());
    for (size_t i = 0; i < blockCache.size(); i++) {
        auto& cache = blockCache.at(i);
        cache.block = Grid::getType(i);
        if (const auto& info = cache.block->getParticlesInfo(); info.has_value()) {
            cache.particles = *info;
        }
    }

    /*if (debug) {
        debug->clear();
        auto iterator = iterate();
        debugIterate(iterator);
    }*/

    {
        particles.clear();
        auto iterator = iterate();
        createParticlesVertices(iterator);

        /*vulkan.dispose(std::move(particles.mesh.vbo));

        if (!particles.vertices.empty()) {
            VulkanBuffer::CreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = particles.vertices.size() * sizeof(ComponentParticles::Vertex);
            bufferInfo.usage =
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
            bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            particles.mesh.vbo = vulkan.createBuffer(bufferInfo);

            vulkan.copyDataToBuffer(particles.mesh.vbo, particles.vertices.data(), bufferInfo.size);

            particles.mesh.instances = particles.vertices.size();
            particles.mesh.count = 10 * 6;

            logger.info("Created particles size: {}", particles.vertices.size());

            particles.vertices.clear();
            particles.vertices.shrink_to_fit();
        }*/
    }

    Grid::RawPrimitiveData map;
    generateMesh(voxelShapeCache, map);

    for (auto& primitive : primitives) {
        vulkan.dispose(std::move(primitive.mesh.ibo));
        vulkan.dispose(std::move(primitive.mesh.vbo));
    }
    primitives.clear();

    for (const auto& [material, data] : map) {
        logger.debug("Building mesh for type: {} of size: {} indices",
                     material->baseColorTexture->getName(),
                     data.indices.size());

        if (data.indices.empty()) {
            continue;
        }

        primitives.emplace_back();
        auto& primitive = primitives.back();

        primitive.material = material;

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = data.vertices.size() * sizeof(VoxelShape::VertexFinal);
        bufferInfo.usage =
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        primitive.mesh.vbo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(primitive.mesh.vbo, data.vertices.data(), bufferInfo.size);

        bufferInfo.size = data.indices.size() * sizeof(uint32_t);
        bufferInfo.usage =
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        primitive.mesh.ibo = vulkan.createBuffer(bufferInfo);
        vulkan.copyDataToBuffer(primitive.mesh.ibo, data.indices.data(), bufferInfo.size);

        primitive.mesh.count = data.indices.size();
        primitive.mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
    }
}

void ComponentGrid::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentGrid>(handle);
}

void ComponentGrid::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentGrid>("ComponentGrid");
    cls["set_from"] = &ComponentGrid::setFrom;
}
