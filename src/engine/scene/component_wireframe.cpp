#include "component_wireframe.hpp"

using namespace Engine;

ComponentWireframe::ComponentWireframe(Engine::Object& object) : Component{object} {
    setDirty(false);
}

ComponentWireframe::ComponentWireframe(Engine::Object& object, std::vector<Vector3> vertices,
                                       std::vector<uint32_t> indices, const Color4& color) :
    Component{object}, vertices{std::move(vertices)}, indices{std::move(indices)}, color{color} {
    setDirty(true);
}

void ComponentWireframe::render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline) {
    /*if (isDirty() && !vertices.empty()) {
        setDirty(false);

        if (!vboFormat) {
            vboFormat = vulkan.createVertexInputFormat({
                {
                    0,
                    {
                        {0, 0, VulkanVertexInputFormat::Format::Vec3},
                    },
                },
            });
        }

        const auto vboSize = vertices.capacity() * sizeof(Vector3);
        if (!vbo || vbo.getSize() != vboSize) {
            vbo = vulkan.createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Dynamic, vboSize);
            vbo.subData(vertices.data(), 0, vboSize);
        } else {
            auto dst = vbo.mapPtr(vboSize);
            std::memcpy(dst, vertices.data(), vboSize);
            vbo.unmap();
        }

        const auto iboSize = indices.capacity() * sizeof(uint32_t);
        if (!ibo || ibo.getSize() != iboSize) {
            ibo = vulkan.createBuffer(VulkanBuffer::Type::Index, VulkanBuffer::Usage::Dynamic, iboSize);
            ibo.subData(indices.data(), 0, iboSize);
        } else {
            auto dst = ibo.mapPtr(vboSize);
            std::memcpy(dst, indices.data(), iboSize);
            ibo.unmap();
        }
    }

    if (!vbo || !ibo || !vboFormat) {
        return;
    }

    vulkan.bindVertexBuffer(vbo, 0);
    vulkan.bindVertexInputFormat(vboFormat);
    vulkan.bindIndexBuffer(ibo, 0, VK_INDEX_TYPE_UINT32);
    vulkan.setInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    vulkan.pushConstant(0, getObject().getAbsoluteTransform());
    vulkan.pushConstant(sizeof(Matrix4), color);
    vulkan.drawIndexed(indices.size(), 1, 0, 0, 0);*/
}

void ComponentWireframe::setBox(float width, const Color4& color) {
    this->color = color;
    const auto half = width / 2.0f;
    const auto start = vertices.size();
    vertices.reserve(vertices.size() + (1024 - vertices.size() % 1024));
    indices.reserve(indices.size() + (1024 - indices.size() % 1024));

    // Bottom
    vertices.push_back(Vector3{-half, -half, -half});
    vertices.push_back(Vector3{-half, -half, half});
    vertices.push_back(Vector3{half, -half, half});
    vertices.push_back(Vector3{half, -half, -half});

    // Top
    vertices.push_back(Vector3{-half, half, -half});
    vertices.push_back(Vector3{-half, half, half});
    vertices.push_back(Vector3{half, half, half});
    vertices.push_back(Vector3{half, half, -half});

    // Bottom
    indices.push_back(start);
    indices.push_back(start + 1);

    indices.push_back(start + 1);
    indices.push_back(start + 2);

    indices.push_back(start + 2);
    indices.push_back(start + 3);

    indices.push_back(start + 3);
    indices.push_back(start);

    // Top
    indices.push_back(start + 4);
    indices.push_back(start + 5);

    indices.push_back(start + 5);
    indices.push_back(start + 6);

    indices.push_back(start + 6);
    indices.push_back(start + 7);

    indices.push_back(start + 7);
    indices.push_back(start + 4);

    // Sides
    indices.push_back(start);
    indices.push_back(start + 4);

    indices.push_back(start + 1);
    indices.push_back(start + 5);

    indices.push_back(start + 2);
    indices.push_back(start + 6);

    indices.push_back(start + 3);
    indices.push_back(start + 7);

    setDirty(true);
}
