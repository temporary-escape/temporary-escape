#include "ComponentWorldText.hpp"
#include <utf8cpp/utf8.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentWorldText::ComponentWorldText(EntityId entity, const FontFace& fontFace, const Color4& color,
                                       const float height) :
    Component{entity}, fontFace{&fontFace}, color{color}, height{height} {
}

void ComponentWorldText::recalculate(VulkanRenderer& vulkan) {
    if (!dirty) {
        return;
    }

    dirty = false;

    logger.debug("Recreating with {} vertices", vertices.size());

    vulkan.dispose(std::move(mesh.vbo));

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    mesh.vbo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(mesh.vbo, vertices.data(), bufferInfo.size);
    mesh.instances = vertices.size();
    mesh.count = 4;
}

void ComponentWorldText::reset() {
    vertices.clear();
    dirty = true;
}

void ComponentWorldText::add(const Vector3& pos, const std::string& text) {
    auto it = text.c_str();
    const auto end = it + text.size();

    auto pen = -fontFace->getPenOffset(text, fontFace->getSize(), textAlign);
    pen += offset;

    const auto scale = (static_cast<float>(height) / static_cast<float>(fontFace->getSize())) * 2.0f;

    while (it < end) {
        const auto code = utf8::next(it, end);
        const auto& glyph = fontFace->getGlyph(code);

        vertices.emplace_back();
        auto& dst = vertices.back();

        dst.position = pos;
        dst.offset = pen + Vector2{0.0f, glyph.ascend * scale + (height - glyph.size.y * scale)};
        dst.size = glyph.size * scale;
        dst.uv = glyph.uv;
        dst.st = glyph.st;

        pen += Vector2{glyph.advance * scale, 0.0f};
    }

    dirty = true;
}
