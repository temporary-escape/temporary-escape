// clang-format off
#include <png.h>
#include "PngImporter.hpp"

#include "Exceptions.hpp"
// clang-format on

using namespace Scissio;

static void user_error_fn(png_structp png_ptr, png_const_charp error_msg) {
    auto str = static_cast<std::string*>(png_get_error_ptr(png_ptr));
    *str = std::string(error_msg);
}

static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
    (void)png_ptr;
    (void)warning_msg;
}

PngImporter::PngImporter(const Path& path) : png{nullptr}, info{nullptr}, file{nullptr} {
    const auto pathStr = path.string();
    file = fopen(pathStr.c_str(), "rb");

    if (!file) {
        EXCEPTION("Failed to open png file: '{}'", path.string());
    }

    std::unique_ptr<uint8_t[]> header(new uint8_t[8]);
    fread(header.get(), 1, 8, file);

    if (png_sig_cmp(header.get(), 0, 8)) {
        fclose(file);
        EXCEPTION("Invalid png file: '{}", path.string());
    }

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, &this->error, user_error_fn, user_warning_fn);

    if (!png) {
        fclose(file);
        EXCEPTION("Failed to read png struct file: '{}' error: png_create_read_struct", path.string());
    }

    info = png_create_info_struct(png);
    if (!info) {
        fclose(file);
        EXCEPTION("Failed to read png info file: '{}' error: png_create_info_struct", path.string());
    }

    if (setjmp(png_jmpbuf(png))) {
        fclose(file);
        EXCEPTION("Failed to read png file: '{}' error: png_jmpbuf", path.string());
    }

    png_init_io(png, file);
    png_set_sig_bytes(png, 8);

    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    depth = 0;

    png_byte colorType = png_get_color_type(png, info);
    png_byte bits = png_get_bit_depth(png, info);

    if (bits == 16)
        png_set_swap(png);

    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (bits == 8) {
        if (colorType == PNG_COLOR_TYPE_RGB) {
            pixelType = PixelType::Rgb8u;
            bpp = 3;
        } else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
            pixelType = PixelType::Rgba8u;
            bpp = 4;
        } else if (colorType == PNG_COLOR_TYPE_PALETTE) {
            pixelType = PixelType::Rgb8u;
            bpp = 3;
        } else {
            fclose(file);
            EXCEPTION("Invalid 8bit png file: '{}'", path.string());
        }

    } else if (bits == 16) {
        if (colorType == PNG_COLOR_TYPE_RGB) {
            pixelType = PixelType::Rgb16u;
            bpp = 6;
        } else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
            pixelType = PixelType::Rgba16u;
            bpp = 8;
        } else {
            fclose(file);
            EXCEPTION("Invalid 16bit png file: '{}'", path.string());
        }

    } else {
        fclose(file);
        EXCEPTION("Invalid png pixel format file: '{}'", path.string());
    }

    size = width * height * bpp;

    png_read_update_info(png, info);

    pixels.reset(new uint8_t[size]);
    auto dst = pixels.get() + static_cast<size_t>(width * (height - 1) * bpp);

    for (auto row = 0; row < height; row++) {
        png_read_row(png, (png_bytep)dst, nullptr);
        dst -= width * bpp;
    }
}

PngImporter::~PngImporter() {
    if (png) {
        png_destroy_read_struct(&png, &info, nullptr);
    }
    if (file) {
        fclose(file);
    }
}

void* PngImporter::getData() const {
    return pixels.get();
}
