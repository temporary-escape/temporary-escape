#pragma once

#include "../Library.hpp"
#include "Component.hpp"
#include "Grid.hpp"

namespace Scissio {
class ShaderGrid;

class SCISSIO_API ComponentGrid : public Component, public Grid {
public:
    static constexpr ComponentType Type = 3;

    struct MeshData {
        uint16_t type{0};
        ModelPtr model{nullptr};
        VertexBuffer instances{NO_CREATE};
        std::list<Primitive> primitives;
    };

    ComponentGrid();
    ComponentGrid(ComponentGrid&& other) noexcept = default;
    ComponentGrid(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(ComponentGrid&& other) noexcept = default;

    explicit ComponentGrid(Object& object);
    virtual ~ComponentGrid() = default;

    const std::vector<MeshData>& getMeshes() const {
        return meshes;
    }

    void render(ShaderGrid& shader);

private:
    void rebuildBuffers();

    std::vector<MeshData> meshes;

public:
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Grid));
};
} // namespace Scissio
