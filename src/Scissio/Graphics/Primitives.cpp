#include "Primitives.hpp"

#include "../Shaders/ShaderGBufferView.hpp"
#include "../Shaders/ShaderSkybox.hpp"
#include "../Shaders/ShaderWireframe.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Scissio;

// Simple skybox box with two triangles per side.
static const float SkyboxVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
};

static const float FullScreenQuad[] = {
    -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
};

static const float WireframeModelBox[] = {
    -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f,

    -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,
    0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,

    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
    0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
};

Mesh Scissio::createSkyboxMesh() {
    Mesh mesh;

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(SkyboxVertices, sizeof(SkyboxVertices), VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), ShaderSkybox::Position{});
    mesh.setCount(6 * 2 * 3);
    mesh.setPrimitive(PrimitiveType::Triangles);

    return mesh;
}

Mesh Scissio::createFullScreenMesh() {
    Mesh mesh;

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(FullScreenQuad, sizeof(FullScreenQuad), VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), ShaderGBufferView::Position{});
    mesh.setCount(6);
    mesh.setPrimitive(PrimitiveType::Triangles);

    return mesh;
}

Mesh Scissio::createWireframeMesh(WireframeModel type) {
    Mesh mesh;

    VertexBuffer vbo(VertexBufferType::Array);

    switch (type) {
    case WireframeModel::Box: {
        vbo.bufferData(WireframeModelBox, sizeof(WireframeModelBox), VertexBufferUsage::StaticDraw);
        mesh.setCount(12 * 2);
        break;
    }
    default: {
        EXCEPTION("Unknown wireframe model: {}", int(type));
    }
    }

    mesh.addVertexBuffer(std::move(vbo), ShaderWireframe::Position{});
    mesh.setPrimitive(PrimitiveType::Lines);

    return mesh;
}
