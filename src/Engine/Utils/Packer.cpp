#include "Packer.hpp"
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

using namespace Engine;

std::vector<Vector4i> Engine::packRectangles(const Vector2i& maxSize, const std::vector<Vector2i>& items,
                                             const int offset) {
    stbrp_context ctx;

    std::vector<stbrp_node> nodes;
    nodes.resize(items.size());
    stbrp_init_target(&ctx, maxSize.x, maxSize.y, nodes.data(), nodes.size());

    std::vector<stbrp_rect> rects;
    rects.resize(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        rects[i].w = static_cast<stbrp_coord>(items[i].x + offset);
        rects[i].h = static_cast<stbrp_coord>(items[i].y + offset);
    }

    const auto ret = stbrp_pack_rects(&ctx, rects.data(), rects.size());
    if (ret > 0) {
        std::vector<Vector4i> results;
        results.resize(items.size());
        for (size_t i = 0; i < items.size(); i++) {
            results[i].x = rects[i].x;
            results[i].y = rects[i].y;
            results[i].z = rects[i].w;
            results[i].w = rects[i].h;
        }

        return results;
    } else {
        return {};
    }
}

struct Packer::Data {
    stbrp_context ctx;
    std::vector<stbrp_node> nodes;
};

Packer::Packer(size_t num, const Vector2i& size) : data(std::make_unique<Data>()) {
    data->nodes.resize(num);
    stbrp_init_target(&data->ctx, size.x, size.y, data->nodes.data(), data->nodes.size());
}

Packer::~Packer() = default;

std::optional<Vector2i> Packer::add(const Vector2i& size) {
    stbrp_rect rect;
    rect.w = size.x;
    rect.h = size.y;

    const auto ret = stbrp_pack_rects(&data->ctx, &rect, 1);
    if (ret > 0) {
        return Vector2i{rect.x, rect.y};
    } else {
        return std::nullopt;
    }
}
