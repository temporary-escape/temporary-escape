#include "ImageLoader.hpp"

#include "Exceptions.hpp"

#include <IL/il.h>

using namespace Scissio;

std::mutex ImageLoader::mutex;

ImageLoader::ImageLoader(const Path& path) {
    std::lock_guard<std::mutex> lock{mutex};
    ilInit();

    ilGenImages(1, &img);
    ilBindImage(img);

    try {
        const auto pathStr = path.string();
        if (!ilLoadImage(pathStr.c_str())) {
            EXCEPTION("Failed to open png file: '{}'", path.string());
        }

        width = ilGetInteger(IL_IMAGE_WIDTH);
        height = ilGetInteger(IL_IMAGE_HEIGHT);
        depth = ilGetInteger(IL_IMAGE_DEPTH);
        const auto bits = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
        const auto format = ilGetInteger(IL_IMAGE_FORMAT);
        size = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);

        pixelType = toPixelType(bits, format);

        if (pixelType == PixelType::None) {
            EXCEPTION("Png file: '{}' has unknown pixel format", path.string());
        }
    } catch (...) {
        ilDeleteImages(1, &img);
        std::rethrow_exception(std::current_exception());
    }
}

ImageLoader::~ImageLoader() {
    if (img != 0) {
        std::lock_guard<std::mutex> lock{mutex};
        ilDeleteImages(1, &img);
    }
}

void* ImageLoader::getData() const {
    std::lock_guard<std::mutex> lock{mutex};
    ilBindImage(img);
    return ilGetData();
}
