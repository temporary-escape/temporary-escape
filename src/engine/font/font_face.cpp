#include "font_face.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/packer.hpp"
#include "font_loader.hpp"
#include <utf8.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

FontFace::FontFace(VulkanRenderer& vulkan, const Path& path, const float size) : size{size} {
    auto loader = FontLoader{path, size};

    std::vector<std::tuple<int, FontLoader::Glyph>> raw;

    static std::vector<std::tuple<int, int>> ranges = {
        {0x0000, 0x0000}, // Unknown,
        {0x0020, 0x007F}, // Basic Latin
        {0x00A0, 0x00FF}, // Latin-1 Supplement
        {0x0100, 0x017F}, // Latin Extended-A
        {0x0180, 0x024F}, // Latin Extended-B
        {0x0250, 0x02AF}, // IPA Extensions
        {0x0400, 0x04FF}, // Cyrillic
        {0x0500, 0x052F}, // Cyrillic Supplementary
        {0x2200, 0x22FF}, // Mathematical Operators
    };

    for (const auto [start, end] : ranges) {
        for (int code = start; code <= end; code++) {
            raw.emplace_back(code, loader.getGlyph(code));
        }
    }

    std::vector<Vector2i> positions{raw.size()};

    int finalWidth = 0;
    int finalHeight = 0;

    for (int width = 512; width <= 8192; width = width << 1) {
        bool failed = false;

        for (int height = width / 2; height <= width; height = height << 1) {
            Packer packer{raw.size(), {width, height}};
            failed = false;

            for (size_t i = 0; i < raw.size(); i++) {
                auto res = packer.add(std::get<1>(raw[i]).bitmapSize + padding * 2);
                if (!res) {
                    failed = true;
                    break;
                }

                positions[i] = *res + padding;
            }

            if (!failed) {
                finalHeight = height;
                break;
            }
        }

        if (!failed) {
            finalWidth = width;
            break;
        }
    }

    if (finalWidth == 0 || finalHeight == 0) {
        EXCEPTION("Unable to pack all glyphs for font: '{}' size: {}", path, size);
    }

    std::unique_ptr<char[]> pixels{new char[finalWidth * finalHeight]};
    auto dst = pixels.get();
    std::memset(pixels.get(), 0x00, finalWidth * finalHeight);

    for (size_t i = 0; i < raw.size(); i++) {
        const auto& data = std::get<1>(raw[i]);
        const auto code = std::get<0>(raw[i]);
        const auto pos = positions[i];

        const auto src = data.bitmap.get();

        for (int row = 0; row < data.bitmapSize.y; row++) {
            for (int col = 0; col < data.bitmapSize.x; col++) {
                const auto x = pos.x + col;
                const auto y = pos.y + row;
                dst[y * finalWidth + x] = src[row * data.bitmapPitch + col];
            }
        }

        Glyph glyph{};
        glyph.advance = static_cast<float>(data.advance >> 6);
        glyph.uv = Vector2{
            static_cast<float>(pos.x) / static_cast<float>(finalWidth),
            static_cast<float>(pos.y) / static_cast<float>(finalHeight),
        };
        glyph.st = Vector2{
            static_cast<float>(data.bitmapSize.x) / static_cast<float>(finalWidth),
            static_cast<float>(data.bitmapSize.y) / static_cast<float>(finalHeight),
        };
        glyph.box = data.box;
        glyph.size = data.bitmapSize;
        glyph.ascend = data.ascend;

        glyphs.insert(std::make_pair(code, glyph));
    }

    unknown = &glyphs.at(0);

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(finalWidth), static_cast<uint32_t>(finalHeight), 1};
    textureInfo.image.mipLevels = getMipMapLevels({finalWidth, finalHeight});
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8_UNORM;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkan.copyDataToImage(texture, 0, {0, 0}, 0, {finalWidth, finalHeight}, pixels.get());

    // vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vulkan.generateMipMaps(texture);

    logger.info("Loaded font: '{}' of size: {} with atlas size: {}", path, size, Vector2i{finalWidth, finalHeight});
}

FontFace::~FontFace() = default;

Vector2 FontFace::getBounds(const std::string_view& text, const float height) const {
    auto it = text.data();
    const auto end = it + text.size();

    size_t total = 0;
    Vector2 max{0.0f};
    Vector2 pos{0.0f};

    const auto scale = height / getSize();

    while (it < end) {
        const auto code = utf8::next(it, end);
        const auto& glyph = getGlyph(code);

        max.y = std::max(max.y, pos.y + height);
        pos += Vector2{glyph.advance * scale, 0.0f};
        max.x = std::max(max.x, pos.x);
    }

    return max;
}
