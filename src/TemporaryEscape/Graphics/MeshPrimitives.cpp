#include "MeshPrimitives.hpp"

using namespace Engine;

static const auto pi = 3.14159265358979323846f;
static const auto pi2 = 1.57079632679489661923f;

Mesh Engine::createFullScreenQuad() {
    struct Pos2UvVertex {
        Vector2 position;
        Vector2 uv;
    };

    static const std::array<Pos2UvVertex, 6> fullscreenQuad = {
        Pos2UvVertex{{-1.0f, -1.0f}, {0.0f, 1.0f}}, Pos2UvVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},
        Pos2UvVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},  Pos2UvVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},
        Pos2UvVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},  Pos2UvVertex{{1.0f, 1.0f}, {1.0f, 0.0f}}};

    auto mesh = Mesh{};

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(fullscreenQuad.data(), fullscreenQuad.size() * sizeof(fullscreenQuad),
                   VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), VertexAttribute<0, Vector2>{}, VertexAttribute<1, Vector2>{});
    mesh.setCount(6);
    mesh.setPrimitive(PrimitiveType::Triangles);

    return mesh;
}

Mesh Engine::createPlanetMesh() {
#pragma pack(push, 1)
    struct Vertex {
        Vector3 pos;
        Vector3 normal;
    };
#pragma pack(pop)

    static_assert(sizeof(Vertex) == 6 * sizeof(float), "Size of Vertex struct must be 6 floats");

    // Source: https://stackoverflow.com/a/5989676
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    static const auto radius = 0.5f;
    static const auto rings = 32;
    static const auto sectors = 64;

    const auto R = 1.0f / static_cast<float>(rings - 1);
    const auto S = 1.0f / static_cast<float>(sectors - 1);

    vertices.resize(rings * sectors);
    indices.resize(rings * sectors * 6);

    auto v = vertices.begin();
    for (auto r = 0; r < rings; r++)
        for (auto s = 0; s < sectors; s++) {
            float const y = std::sin(-pi2 + pi * r * R);
            float const x = std::cos(2 * pi * s * S) * std::sin(pi * r * R);
            float const z = std::sin(2 * pi * s * S) * std::sin(pi * r * R);

            *v++ = Vertex{Vector3{x, y, z} * radius, Vector3{x, y, z}};
        }

    auto i = indices.begin();
    for (auto r = 0; r < rings; r++) {
        for (auto s = 0; s < sectors; s++) {
            *i++ = r * sectors + s;
            *i++ = r * sectors + (s + 1);
            *i++ = (r + 1) * sectors + (s + 1);

            *i++ = r * sectors + s;
            *i++ = (r + 1) * sectors + (s + 1);
            *i++ = (r + 1) * sectors + s;
        }
    }

    auto mesh = Mesh{};

    auto vbo = VertexBuffer(VertexBufferType::Array);
    vbo.bufferData(vertices.data(), vertices.size() * sizeof(Vertex), VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), VertexAttribute<0, Vector3>{}, VertexAttribute<1, Vector3>{});

    auto ibo = VertexBuffer(VertexBufferType::Indices);
    ibo.bufferData(indices.data(), indices.size() * sizeof(uint16_t), VertexBufferUsage::StaticDraw);

    mesh.setIndexBuffer(std::move(ibo), IndexType::UnsignedShort);

    mesh.setCount(static_cast<int>(indices.size()));
    mesh.setPrimitive(PrimitiveType::Triangles);

    return mesh;
}

Mesh Engine::createSkyboxMesh() {
    static const float skyboxVertices[] = {
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

    auto mesh = Mesh{};

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(skyboxVertices, sizeof(skyboxVertices), VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), VertexAttribute<0, Vector3>{});
    mesh.setCount(6 * 2 * 3);
    mesh.setPrimitive(PrimitiveType::Triangles);

    return mesh;
}
