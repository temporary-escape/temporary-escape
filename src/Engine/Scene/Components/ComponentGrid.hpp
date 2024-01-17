#pragma once

#include "../../Assets/Primitive.hpp"
#include "../../Assets/ShipTemplate.hpp"
#include "../Component.hpp"
#include "../Grid.hpp"
#include "ComponentDebug.hpp"
#include "ComponentParticles.hpp"
#include "ComponentTransform.hpp"

class btCollisionShape;
class btCompoundShape;

namespace Engine {
class ENGINE_API ComponentGrid : public Component, public Grid {
public:
    using ParticlesMap = std::unordered_map<ParticlesTypePtr, std::vector<Matrix4>>;

    ComponentGrid();
    explicit ComponentGrid(EntityId entity);
    virtual ~ComponentGrid(); // NOLINT(*-use-override)
    ComponentGrid(const ComponentGrid& other) = delete;
    ComponentGrid(ComponentGrid&& other) noexcept;
    ComponentGrid& operator=(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(ComponentGrid&& other) noexcept;
    static constexpr auto in_place_delete = true;

    void setFrom(const ShipTemplatePtr& shipTemplate);

    void clear();
    void recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache);

    [[nodiscard]] const std::list<Primitive>& getPrimitives() const {
        return primitives;
    }

    [[nodiscard]] const ParticlesMap& getParticles() const {
        return particles;
    }

    [[nodiscard]] std::unique_ptr<btCollisionShape> createCollisionShape();

    void setDirty() {
        dirty = true;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Grid));

private:
    struct BlockCache {
        BlockPtr block;
        Block::Definition::ParticlesInfo particles;
    };

    void debugIterate(Grid::Iterator iterator);
    void createParticlesVertices(Grid::Iterator iterator);
    void createShape(btCompoundShape& compoundShape, Grid::Iterator iterator) const;

    bool dirty{false};
    ComponentDebug* debug{nullptr};
    VulkanRenderer* vulkanRenderer{nullptr};
    std::list<Primitive> primitives;
    std::vector<BlockCache> blockCache;
    ParticlesMap particles;
};
} // namespace Engine
