#include "ComponentGrid.hpp"

using namespace Scissio;

ComponentGrid::ComponentGrid(Object& object) : Component(object) {
}

void ComponentGrid::rebuildBuffers() {
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, Vector3> Normal;
    typedef VertexAttribute<2, Vector2> TextureCoordinates;
    typedef VertexAttribute<3, Vector4> Tangent;
    typedef VertexAttribute<4, Matrix4> Instances;

    const auto blocks = Grid::buildInstanceBuffer();

    const auto fastInsert = meshes.empty();

    const auto nextMeshData = [this](const BlockInstances& block) -> MeshData& {
        meshes.emplace_back();
        meshes.back().model = block.model;
        meshes.back().type = block.type;
        return meshes.back();
    };

    const auto findMeshData = [this, &nextMeshData](const BlockInstances& block) -> MeshData& {
        const auto it = std::find_if(meshes.begin(), meshes.end(), [&](MeshData& m) { return block.type == m.type; });
        if (it == meshes.end()) {
            return nextMeshData(block);
        }

        return *it;
    };

    for (const auto& block : blocks) {
        const auto& model = block.model;
        const auto& matrices = block.instances;

        if (matrices.empty()) {
            meshes.erase(
                std::remove_if(meshes.begin(), meshes.end(), [&](MeshData& m) { return m.type == block.type; }),
                meshes.end());
            continue;
        }

        auto& data = fastInsert ? nextMeshData(block) : findMeshData(block);

        data.instances = VertexBuffer(VertexBufferType::Array);
        data.primitives.clear();

        data.instances.bufferData(&matrices[0][0].x, sizeof(Matrix4) * matrices.size(), VertexBufferUsage::StaticDraw);

        for (const auto& primitive : model->getPrimitives()) {
            data.primitives.emplace_back();
            auto& p = data.primitives.back();
            auto& mesh = p.mesh;
            p.material = primitive.material;

            mesh = Mesh{};
            mesh.addVertexBuffer(primitive.vbo, Position{}, Normal{}, TextureCoordinates{}, Tangent{});
            mesh.addVertexBufferInstanced(data.instances, Instances{});
            mesh.setIndexBuffer(primitive.ibo, primitive.mesh.getIndexType());
            mesh.setPrimitive(primitive.mesh.getPrimitive());
            mesh.setCount(primitive.mesh.getCount());
            mesh.setInstancesCount(static_cast<GLsizei>(matrices.size()));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}
