#include "WorldSpaceText.hpp"
#include "../Font/FontFamily.hpp"
#include <utf8/checked.h>

using namespace Engine;

WorldSpaceText::TextVertexShaper::TextVertexShaper(Vertices& vertices, const FontFamily& font, const float size,
                                                   const Vector3& pos) :
    TextShaper{font, size}, vertices{vertices}, pos{pos} {
}

void WorldSpaceText::TextVertexShaper::onGlyph(const FontFace& fontFace, const FontFace::Glyph& glyph,
                                               const Vector2& pen, const Quad& quad) {
    vertices.emplace_back();
    auto& dst = vertices.back();

    const auto height = static_cast<float>(fontFace.getSize());

    dst.position = pos;
    dst.offset = pen + Vector2{0.0f, glyph.ascend * getScale() + (height - glyph.size.y * getScale())};
    dst.offset.y -= (height * getScale());
    dst.size = glyph.size * getScale();
    dst.uv = glyph.uv;
    dst.st = glyph.st;
}

WorldSpaceText::WorldSpaceText(const FontFamily& font) : font{font} {
}

void WorldSpaceText::add(const Vector3& pos, const std::string_view& text, const float size) {
    const auto start = vertices.size();
    vertices.reserve(vertices.size() + utf8::distance(text.begin(), text.end()));

    TextVertexShaper shaper{vertices, font, size, pos};
    shaper.write(text);

    const auto bounds = shaper.getBounds() * textAlignToVector(textAlign);
    for (size_t i = start; i < vertices.size(); i++) {
        vertices[i].offset -= bounds;
    }
}

void WorldSpaceText::recalculate(VulkanRenderer& vulkan) {
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

void WorldSpaceText::clear() {
    vertices.clear();
}
