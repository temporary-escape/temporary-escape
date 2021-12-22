#pragma once

#include "../Library.hpp"
#include "Component.hpp"
#include "Grid.hpp"

namespace Scissio {
class SCISSIO_API ComponentGrid : public Component, public Grid {
public:
    struct MeshData {
        uint16_t type{0};
        AssetModelPtr model{nullptr};
        VertexBuffer instances{NO_CREATE};
        std::list<Primitive> primitives;
    };

    ComponentGrid() = default;
    ComponentGrid(ComponentGrid&& other) noexcept = default;
    ComponentGrid(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(ComponentGrid&& other) noexcept = default;

    explicit ComponentGrid(Object& object);
    virtual ~ComponentGrid() = default;

    const std::vector<MeshData>& getMeshes() const {
        return meshes;
    }

private:
    void rebuildBuffers();

    std::vector<MeshData> meshes;

public:
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Grid));
};
} // namespace Scissio
