#include "ComponentGrid.hpp"
#include "../../File/MsgpackFileReader.hpp"
#include "../../File/TebFileHeader.hpp"
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static auto boxShape = std::make_unique<btBoxShape>(btVector3{0.5f, 0.5f, 0.5f});

ComponentGrid::ComponentGrid() = default;

ComponentGrid::ComponentGrid(EntityId entity) : Component{entity} {
}

ComponentGrid::~ComponentGrid() {
    clear();
}

ComponentGrid::ComponentGrid(ComponentGrid&& other) noexcept = default;

ComponentGrid& ComponentGrid::operator=(ComponentGrid&& other) noexcept = default;

void ComponentGrid::clear() {
    if (vulkanRenderer) {
        vulkanRenderer->dispose(std::move(mesh));
    }
}

std::unique_ptr<btCollisionShape> ComponentGrid::createCollisionShape() {
    if (pool().size() == 0) {
        return nullptr;
    }

    auto shape = std::make_unique<btCompoundShape>();

    auto iterator = iterate();
    createShape(*shape, iterator);

    shape->recalculateLocalAabb();

    return shape;
}

void ComponentGrid::createShape(btCompoundShape& compoundShape, Grid::Iterator iterator) const {
    while (iterator) {
        if (iterator.isVoxel()) {
            auto pos = iterator.getPos();
            btTransform transform{};
            transform.setIdentity();
            transform.setOrigin({
                static_cast<float>(pos.x),
                static_cast<float>(pos.y),
                static_cast<float>(pos.z),
            });
            compoundShape.addChildShape(transform, boxShape.get());
        } else {
            createShape(compoundShape, iterator.children());
        }

        iterator.next();
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
    updateBounds();

    dirty = true;
}

/*void ComponentGrid::debugIterate(Grid::Iterator iterator) {
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
}*/

void ComponentGrid::recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache) {
    if (!dirty) {
        return;
    }

    dirty = false;

    vulkanRenderer = &vulkan;

    blockCache.clear();
    blockCache.resize(Grid::getTypeCount());
    for (size_t i = 0; i < blockCache.size(); i++) {
        auto& cache = blockCache.at(i);
        cache.block = Grid::getType(i);
    }

    Grid::BlocksData data;
    generateMesh(voxelShapeCache, data);

    thrusters = std::move(data.thrusters);

    if (mesh) {
        vulkan.dispose(std::move(mesh));
    }

    // logger.debug("Building mesh of size: {} indices", data.indices.size());

    if (data.indices.empty()) {
        return;
    }

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = data.vertices.size() * sizeof(VoxelShape::VertexFinal);
    bufferInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, data.vertices.data(), bufferInfo.size);

    bufferInfo.size = data.indices.size() * sizeof(uint32_t);
    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    mesh.ibo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.ibo, data.indices.data(), bufferInfo.size);

    mesh.count = data.indices.size();
    mesh.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
}
