#include "Canvas2.hpp"
#include <utf8cpp/utf8.h>

using namespace Engine;

Canvas2::Canvas2(VulkanRenderer& vulkan) : vulkan{vulkan} {
    resetTextures();
}

Canvas2::~Canvas2() {
    vulkan.dispose(std::move(vbo));
    vulkan.dispose(std::move(ibo));
    vulkan.dispose(std::move(cbo));
}

/*void Canvas2::render(VulkanCommandBuffer& vkb) {
    flush(vertices, vbo);
    flush(indices, ibo);
    flush(commands, cbo);

    std::array<VulkanVertexBufferBindRef, 1> vbos{};
    vbos[0] = {&vbo.getCurrentBuffer(), 0};
    vkb.bindBuffers(vbos);
    vkb.bindIndexBuffer(ibo.getCurrentBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkb.drawIndirect(cbo.getCurrentBuffer(), 0, commands.count, sizeof(VkDrawIndirectCommand));

    vertices.count = 0;
    indices.count = 0;
    commands.count = 0;
}*/

void Canvas2::flush() {
    flush(vertices, vbo);
    flush(indices, ibo);
    flush(commands, cbo);
}

void Canvas2::begin(const Vector2i& viewport) {
    vertices.count = 0;
    indices.count = 0;
    commands.count = 0;
    currentViewport = viewport;

    batches.clear();

    for (auto& texture : textures) {
        texture = nullptr;
    }

    auto& batch = batches.emplace_back();
    batch.offset = 0;
    batch.length = 0;
    batch.scissor = {
        {0, 0},
        {static_cast<int>(viewport.x), static_cast<int>(viewport.y)},
    };
}

void Canvas2::setScissor(const Vector2& pos, const Vector2& size) {
    /*if (batches.back().length) {
        const auto newOffset = batches.back().offset + batches.back().length;

        auto& batch = batches.emplace_back();
        batch.offset = newOffset;
        batch.length = 0;
    }

    batches.back().scissor = {
        {static_cast<int>(pos.x), static_cast<int>(pos.x)},
        {static_cast<int>(size.x), static_cast<int>(size.y)},
    };*/
}

void Canvas2::clearScissor() {
    setScissor({0.0f, 0.0f}, currentViewport);
}

void Canvas2::doRect(Canvas2::Vertex*& v, uint32_t*& i, const Vector2& pos, const Vector2& size, const Color4& color) {
    v[0].pos = pos;
    v[1].pos = pos + Vector2{size.x, 0.0f};
    v[2].pos = pos + Vector2{size.x, size.y};
    v[3].pos = pos + Vector2{0.0f, size.y};

    v[0].color = color;
    v[1].color = color;
    v[2].color = color;
    v[3].color = color;

    v[0].uv = {0.0f, 0.0f, 0.0f, 0.0f};
    v[1].uv = {0.0f, 0.0f, 0.0f, 0.0f};
    v[2].uv = {0.0f, 0.0f, 0.0f, 0.0f};
    v[3].uv = {0.0f, 0.0f, 0.0f, 0.0f};

    const auto offset = getOffset(vertices, v);
    i[0] = offset;
    i[1] = offset + 1;
    i[2] = offset + 2;
    i[3] = offset + 2;
    i[4] = offset + 3;
    i[5] = offset;

    v += 4;
    i += 6;
}

void Canvas2::drawRect(const Vector2& pos, const Vector2& size, const Color4& color) {
    auto* v = reserve(vertices, 4);
    auto* i = reserve(indices, 6);
    auto* c = reserve(commands, 1);

    c->firstIndex = getOffset(indices, i);
    c->firstInstance = 0;
    c->instanceCount = 1;
    c->vertexOffset = 0;
    c->indexCount = 6;

    doRect(v, i, pos, size, color);

    batches.back().length++;
}

void Canvas2::drawRectOutline(const Vector2& pos, const Vector2& size, float thickness, const Color4& color) {
    auto* v = reserve(vertices, 4 * 4);
    auto* i = reserve(indices, 6 * 4);
    auto* c = reserve(commands, 1);

    c->firstIndex = getOffset(indices, i);
    c->firstInstance = 0;
    c->instanceCount = 1;
    c->vertexOffset = 0;
    c->indexCount = 6 * 4;

    doRect(v, i, Vector2{pos.x, pos.y}, Vector2{size.x - thickness, thickness}, color);
    doRect(v, i, Vector2{pos.x + size.x - thickness, pos.y}, Vector2{thickness, size.y - thickness}, color);
    doRect(v, i, Vector2{pos.x + thickness, pos.y + size.y - thickness}, Vector2{size.x - thickness, thickness}, color);
    doRect(v, i, Vector2{pos.x, pos.y + thickness}, Vector2{thickness, size.y - thickness}, color);

    batches.back().length++;
}

void Canvas2::drawTexture(const Vector2& pos, const Vector2& size, const VulkanTexture& texture, const Color4& color) {
    auto* v = reserve(vertices, 4);
    auto* i = reserve(indices, 6);
    auto* c = reserve(commands, 1);
    const auto tex = static_cast<float>(addTexture(texture));

    v[0].pos = pos;
    v[1].pos = pos + Vector2{size.x, 0.0f};
    v[2].pos = pos + Vector2{size.x, size.y};
    v[3].pos = pos + Vector2{0.0f, size.y};

    v[0].color = color;
    v[1].color = color;
    v[2].color = color;
    v[3].color = color;

    v[0].uv = {0.0f, 0.0f, 1.0f, tex};
    v[1].uv = {1.0f, 0.0f, 1.0f, tex};
    v[2].uv = {1.0f, 1.0f, 1.0f, tex};
    v[3].uv = {0.0f, 1.0f, 1.0f, tex};

    const auto offset = getOffset(vertices, v);
    i[0] = offset;
    i[1] = offset + 1;
    i[2] = offset + 2;
    i[3] = offset + 2;
    i[4] = offset + 3;
    i[5] = offset;

    c->firstIndex = getOffset(indices, i);
    c->firstInstance = 0;
    c->instanceCount = 1;
    c->vertexOffset = 0;
    c->indexCount = 6;

    batches.back().length++;
}

void Canvas2::drawText(const Vector2& pos, const std::string_view& text, const FontFamily& font, const int size,
                       const Color4& color) {
    const auto total = utf8::distance(text.begin(), text.end());
    const auto& face = font.get(FontFace::Regular);

    auto* v = reserve(vertices, 4 * total);
    auto* i = reserve(indices, 6 * total);
    auto* c = reserve(commands, 1);
    const auto tex = static_cast<float>(addTexture(face.getTexture()));

    c->firstIndex = getOffset(indices, i);
    c->firstInstance = 0;
    c->instanceCount = 1;
    c->vertexOffset = 0;
    c->indexCount = 6 * total;

    auto pen = pos;

    const auto scale = static_cast<float>(size) / static_cast<float>(face.getSize());

    auto it = text.begin();
    while (it < text.end()) {
        const auto code = utf8::next(it, text.end());
        const auto& glyph = face.getGlyph(code);

        const auto p = pen + Vector2{0.0f, glyph.ascend * scale};

        v[0].pos = p + Vector2{0.0f, -glyph.size.y * scale};
        v[0].color = color;
        v[0].uv = Vector4{glyph.uv.x, glyph.uv.y, 2.0f, tex};

        v[1].pos = p + Vector2{glyph.size.x * scale, -glyph.size.y * scale};
        v[1].color = color;
        v[1].uv = Vector4{glyph.uv.x + glyph.st.x, glyph.uv.y, 2.0f, tex};

        v[2].pos = p + Vector2{glyph.size.x * scale, 0.0f};
        v[2].color = color;
        v[2].uv = Vector4{glyph.uv.x + glyph.st.x, glyph.uv.y + glyph.st.y, 2.0f, tex};

        v[3].pos = p;
        v[3].color = color;
        v[3].uv = Vector4{glyph.uv.x, glyph.uv.y + glyph.st.y, 2.0f, tex};

        const auto offset = getOffset(vertices, v);
        i[0] = offset;
        i[1] = offset + 1;
        i[2] = offset + 2;
        i[3] = offset + 2;
        i[4] = offset + 3;
        i[5] = offset;

        v += 4;
        i += 6;

        pen += Vector2{glyph.advance * scale, 0.0f};
    }

    batches.back().length++;
}

void Canvas2::drawImage(const Vector2& pos, const Vector2& size, const Image& image, const Color4& color) {
    if (!image.getAllocation().texture) {
        return;
    }

    const auto& alc = image.getAllocation();

    auto* v = reserve(vertices, 4);
    auto* i = reserve(indices, 6);
    auto* c = reserve(commands, 1);
    const auto tex = static_cast<float>(addTexture(*alc.texture));

    v[0].pos = pos;
    v[1].pos = pos + Vector2{size.x, 0.0f};
    v[2].pos = pos + Vector2{size.x, size.y};
    v[3].pos = pos + Vector2{0.0f, size.y};

    v[0].color = color;
    v[1].color = color;
    v[2].color = color;
    v[3].color = color;

    v[0].uv = {alc.uv.x, alc.uv.y, 1.0f, tex};
    v[1].uv = {alc.uv.x + alc.st.x, alc.uv.y, 1.0f, tex};
    v[2].uv = {alc.uv.x + alc.st.x, alc.uv.y + alc.st.y, 1.0f, tex};
    v[3].uv = {alc.uv.x, alc.uv.y + alc.st.y, 1.0f, tex};

    const auto offset = getOffset(vertices, v);
    i[0] = offset;
    i[1] = offset + 1;
    i[2] = offset + 2;
    i[3] = offset + 2;
    i[4] = offset + 3;
    i[5] = offset;

    c->firstIndex = getOffset(indices, i);
    c->firstInstance = 0;
    c->instanceCount = 1;
    c->vertexOffset = 0;
    c->indexCount = 6;

    batches.back().length++;
}

template <typename T> size_t Canvas2::getOffset(Buffer<T>& buffer, const T* ptr) const {
    return ptr - &buffer.data.front();
}

template <typename T> T* Canvas2::reserve(Buffer<T>& buffer, const size_t count) {
    if (buffer.count + count > buffer.data.size()) {
        buffer.data.resize(buffer.data.size() + 1024);
    }

    auto* dst = &buffer.data.at(buffer.count);
    buffer.count += count;
    return dst;
}

template <> VkBufferUsageFlags Canvas2::getBufferUsage<Canvas2::Vertex>() const {
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
}

template <> VkBufferUsageFlags Canvas2::getBufferUsage<uint32_t>() const {
    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
}

template <> VkBufferUsageFlags Canvas2::getBufferUsage<VkDrawIndexedIndirectCommand>() const {
    return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
}

template <typename T> void Canvas2::flush(Buffer<T>& buffer, VulkanDoubleBuffer& target) {
    const auto expectedSize = sizeof(T) * buffer.data.size();

    if (!target || target.getSize() != expectedSize) {
        if (target) {
            vulkan.dispose(std::move(target));
        }

        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = expectedSize;
        bufferInfo.usage = getBufferUsage<T>();
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        if (expectedSize != 0) {
            target = vulkan.createDoubleBuffer(bufferInfo);
        }
    }

    if (expectedSize != 0) {
        auto* dst = target.getCurrentBuffer().getMappedPtr();
        std::memcpy(dst, buffer.data.data(), expectedSize);
    }
}

void Canvas2::resetTextures() {
    /*for (size_t i = 0; i < FontFamily::total; i++) {
        textures[i] = &fontFamily.get(static_cast<FontFace::Type>(i)).getTexture();
    }*/
    for (size_t i = 0; i < textures.size(); i++) {
        textures[i] = nullptr;
    }
}

size_t Canvas2::addTexture(const VulkanTexture& texture) {
    for (size_t i = 0; i < textures.size(); i++) {
        if (textures[i] == &texture) {
            return i;
        }

        if (textures[i] == nullptr) {
            textures[i] = &texture;
            return i;
        }
    }

    EXCEPTION("Canvas has too many assigned textures");
}
