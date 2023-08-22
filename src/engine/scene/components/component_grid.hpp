#pragma once

#include "../../assets/primitive.hpp"
#include "../component.hpp"
#include "../grid.hpp"
#include "component_debug.hpp"
#include "component_particles.hpp"

namespace Engine {
class ENGINE_API ComponentGrid : public Component, public Grid {
public:
    struct BlockCache {
        BlockPtr block{nullptr};
        Block::ParticleInfo particles;
    };

    ComponentGrid() = default;
    explicit ComponentGrid(entt::registry& reg, entt::entity handle);
    virtual ~ComponentGrid(); // NOLINT(*-use-override)
    COMPONENT_DEFAULTS(ComponentGrid);

    void clear();
    void recalculate(VulkanRenderer& vulkan, const VoxelShapeCache& voxelShapeCache);
    void update();

    [[nodiscard]] const std::list<Primitive>& getPrimitives() const {
        return primitives;
    }

    [[nodiscard]] const Mesh& getParticlesMesh() const {
        return particles.mesh;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Grid));

private:
    void debugIterate(Grid::Iterator iterator);
    void createParticlesVertices(Grid::Iterator iterator);

    ComponentDebug* debug{nullptr};
    VulkanRenderer* vulkanRenderer{nullptr};
    std::list<Primitive> primitives;
    std::vector<BlockCache> blockCache;

    struct {
        Mesh mesh;
        std::vector<ComponentParticles::Vertex> vertices;
    } particles;
};
} // namespace Engine
