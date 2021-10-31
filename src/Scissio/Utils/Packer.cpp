#include "Packer.hpp"
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

using namespace Scissio;

std::vector<Vector4i> Scissio::packRectangles(const Vector2i& maxSize, const std::vector<Vector2i>& items,
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
