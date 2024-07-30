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

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

    [[nodiscard]] std::unique_ptr<btCollisionShape> createCollisionShape();

    [[nodiscard]] const std::vector<Grid::ThrusterInfo>& getThrusters() const {
        return thrusters;
    }

    void setDirty() {
        dirty = true;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Grid));

private:
    struct BlockCache {
        BlockPtr block;
    };

    // void debugIterate(Grid::Iterator iterator);
    void createShape(btCompoundShape& compoundShape, Grid::Iterator iterator) const;

    bool dirty{false};
    VulkanRenderer* vulkanRenderer{nullptr};
    Mesh mesh;
    std::vector<BlockCache> blockCache;
    std::vector<Grid::ThrusterInfo> thrusters;
};
} // namespace Engine
