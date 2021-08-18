#include "BasicTexture.hpp"
#include "../Utils/PngImporter.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

BasicTexture::BasicTexture(const Manifest& mod, std::string name, const Path& path)
    : Asset(mod, std::move(name)), path(path), texture{NO_CREATE} {
}

void BasicTexture::load(AssetManager& assetManager) {
    Log::d("Loading basic texture: '{}'", getName());

    try {
        PngImporter image(path);

        auto size = image.getSize();

        texture = Texture2D{};
        texture.setStorage(0, size, image.getPixelType());
        texture.setPixels(0, {0, 0}, image.getSize(), image.getPixelType(), image.getData());

    } catch (...) {
        EXCEPTION_NESTED("Failed to load pbr texture: '{}'", getName());
    }
}

MSGPACK_UNPACK_FUNC(BasicTexturePtr) {
    if (o.type == msgpack::type::STR) {
        const auto name = o.as<std::string>();
        v = AssetManager::singleton().find<BasicTexture>(name);
    } else {
        throw msgpack::type_error();
    }

    return o;
}

MSGPACK_PACK_FUNC(BasicTexturePtr) {
    if (v) {
        const auto& name = v->getName();
        o.pack_str(static_cast<uint32_t>(name.size()));
        o.pack_str_body(name.c_str(), static_cast<uint32_t>(name.size()));
    } else {
        o.pack_nil();
    }

    return o;
}
