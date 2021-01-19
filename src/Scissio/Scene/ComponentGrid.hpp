#pragma once

#include "../Library.hpp"
#include "../Shaders/ShaderGrid.hpp"
#include "ComponentSystem.hpp"
#include "Grid.hpp"

namespace Scissio {
class Renderer;

class SCISSIO_API ComponentGrid : public Component, public Grid {
public:
    static constexpr ComponentType Type = 3;

    struct MeshData {
        VertexBuffer instances{NO_CREATE};
        std::list<Primitive> primitives;
    };

    ComponentGrid() = default;
    ComponentGrid(ComponentGrid&& other) = default;
    ComponentGrid(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(const ComponentGrid& other) = delete;
    ComponentGrid& operator=(ComponentGrid&& other) = default;

    explicit ComponentGrid(Object& object) : Component(Type, object) {
    }
    virtual ~ComponentGrid() = default;

    void rebuildBuffers() {
        const auto buffers = Grid::buildInstanceBuffer();

        for (const auto& pair : buffers) {
            const auto& block = pair.first;
            const auto& matrices = pair.second;

            auto& data = meshes[block];
            data.instances = VertexBuffer(VertexBufferType::Array);
            data.primitives.clear();

            data.instances.bufferData(&matrices[0][0].x, sizeof(Matrix4) * matrices.size(),
                                      VertexBufferUsage::StaticDraw);

            for (auto& primitive : block->getModel()->getPrimitives()) {
                data.primitives.emplace_back();
                auto& p = data.primitives.back();
                auto& mesh = p.mesh;
                p.material = primitive.material;

                mesh = Mesh{};
                mesh.addVertexBuffer(primitive.vbo, ShaderModel::Position{}, ShaderModel::Normal{},
                                     ShaderModel::TextureCoordinates{}, ShaderModel::Tangent{});
                mesh.addVertexBufferInstanced(data.instances, ShaderGrid::Instances{});
                mesh.setIndexBuffer(primitive.ibo, primitive.mesh.getIndexType());
                mesh.setPrimitive(primitive.mesh.getPrimitive());
                mesh.setCount(primitive.mesh.getCount());
                mesh.setInstancesCount(matrices.size());

                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
        }
    }

    const std::unordered_map<BlockPtr, MeshData>& getMeshes() const {
        return meshes;
    }

private:
    std::unordered_map<BlockPtr, MeshData> meshes;
};
} // namespace Scissio
