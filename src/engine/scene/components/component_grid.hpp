#pragma once

#include "../../assets/primitive.hpp"
#include "../../assets/ship_template.hpp"
#include "../component.hpp"
#include "../grid.hpp"
#include "component_debug.hpp"
#include "component_particles.hpp"
#include "component_transform.hpp"

class btCollisionShape;
class btCompoundShape;

namespace Engine {
class ENGINE_API ComponentGrid : public Component, public Grid {
public:
    using ParticlesMap = std::unordered_map<ParticlesTypePtr, std::vector<Matrix4>>;

    ComponentGrid();
    explicit ComponentGrid(entt::registry& reg, entt::entity handle);
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

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Grid));

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    struct BlockCache {
        BlockPtr block;
        Block::Definition::ParticlesInfo particles;
    };

    void debugIterate(Grid::Iterator iterator);
    void createParticlesVertices(Grid::Iterator iterator);
    void createShape(btCompoundShape& compoundShape, Grid::Iterator iterator) const;

    ComponentDebug* debug{nullptr};
    VulkanRenderer* vulkanRenderer{nullptr};
    std::list<Primitive> primitives;
    std::vector<BlockCache> blockCache;
    ParticlesMap particles;
};
} // namespace Engine
