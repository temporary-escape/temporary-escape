#include "ComponentGrid.hpp"
#include "../Shaders/ShaderGrid.hpp"

#define CMP "ComponentGrid"

using namespace Engine;

void ComponentGrid::update() {
}

void ComponentGrid::recalculate(Grid::Builder& gridBuilder) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    Grid::Builder::RawPrimitiveData map;
    generateMesh(gridBuilder, map);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    primitives.clear();

    for (const auto& [material, data] : map) {
        Log::d(CMP, "Building mesh for type: {} of size: {} indices", material->baseColorTexture->getName(),
               data.indices.size());

        if (data.indices.empty()) {
            continue;
        }

        primitives.emplace_back();
        auto& primitive = primitives.back();

        primitive.mesh = Mesh{};

        primitive.ibo = VertexBuffer(VertexBufferType::Indices);
        primitive.ibo.bufferData(data.indices.data(), data.indices.size() * sizeof(uint32_t),
                                 VertexBufferUsage::StaticDraw);
        primitive.mesh.setIndexBuffer(primitive.ibo, IndexType::UnsignedInt);

        primitive.vbo = VertexBuffer(VertexBufferType::Array);
        primitive.vbo.bufferData(data.vertices.data(), data.vertices.size() * sizeof(Shape::Vertex),
                                 VertexBufferUsage::StaticDraw);

        primitive.mesh.addVertexBuffer(primitive.vbo, ShaderGrid::Position{}, ShaderGrid::Normal{},
                                       ShaderGrid::Tangent{});

        primitive.mesh.setPrimitive(PrimitiveType::Triangles);
        primitive.mesh.setCount(static_cast<GLint>(data.indices.size()));

        primitive.material = *material;
        primitive.ubo = VertexBuffer(VertexBufferType::Uniform);
        primitive.ubo.bufferData(&primitive.material.uniform, sizeof(Material::Uniform), VertexBufferUsage::StaticDraw);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
