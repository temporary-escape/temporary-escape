#include "AssetManager.hpp"

#include "../Utils/Exceptions.hpp"
#include "../Utils/GltfImporter.hpp"
#include "../Utils/StringUtils.hpp"
#include "Block.hpp"
#include "FontFamily.hpp"
#include "IconAtlas.hpp"
#include "Model.hpp"
#include "PbrTexture.hpp"

#include <fstream>
#include <iostream>
#include <set>

using namespace Scissio;

AssetManager* AssetManager::instance = nullptr;

static const std::array<GLenum, 6> CUBEMAP_ENUMS = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

static std::unique_ptr<uint8_t[]> pixelsFromColor(const Vector2i& size, const Color4& color) {
    std::unique_ptr<uint8_t[]> pixels(new uint8_t[size.x * size.y * 3]);
    for (size_t i = 0; i < size.x * size.y * 3; i += 3) {
        pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
        pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
        pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
    }
    return pixels;
}

AssetManager::AssetManager(const Config& config, Canvas2D& canvas, TextureCompressor& textureCompressor,
                           Renderer& renderer, SkyboxRenderer& skyboxRenderer)
    : canvas(canvas), imageAtlas(config), textureCompressor(textureCompressor), renderer(renderer) {

    instance = this;

    // Generate skybox for thumbnails
    TextureCubemap skybox;
    skybox.bind();

    const auto pixels = pixelsFromColor({64, 64}, {0.07f, 0.07f, 0.07f, 1.0f});
    for (const auto& side : CUBEMAP_ENUMS) {
        glTexImage2D(side, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
    }

    skybox.texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    skybox.texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    skybox.texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    skybox.texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    skybox.texParameteri(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    skybox.texParameteri(GL_TEXTURE_BASE_LEVEL, 0);
    skybox.texParameteri(GL_TEXTURE_MAX_LEVEL, 0);

    auto prefilter = skyboxRenderer.prefilter(skybox);
    auto irradiance = skyboxRenderer.irradiance(skybox);

    thumbnailSkybox = Skybox{
        std::move(skybox),
        std::move(prefilter),
        std::move(irradiance),
    };

    // Configure thumbnail FBO
    thumbnailFboColor.setStorage(0, {128, 128}, PixelType::Rgba8u);
    thumbnailFboDepth.setStorage({128, 128}, PixelType::Depth24Stencil8);
    thumbnailFbo.attach(thumbnailFboColor, FramebufferAttachment::Color0, 0);
    thumbnailFbo.attach(thumbnailFboDepth, FramebufferAttachment::DepthStencil);
}

AssetManager& AssetManager::singleton() {
    if (!instance) {
        EXCEPTION("Asset manager has not been initialized");
    }
    return *instance;
}

template <> FontFacePtr AssetManager::load<FontFace>(const Manifest& mod, const Path& path) {
    static const std::set<std::string> SUFFIXES = {
        "bold",          "bolditalic", "light",          "lightitalic", "regular",
        "regularitalic", "semibold",   "semibolditalic", "thin",        "thinitalic",
    };

    try {
        const auto baseName = path.stem().string();
        const auto tokens = splitLast(baseName, "-");
        if (tokens.size() != 2) {
            EXCEPTION("Font '{}' has incorrect name, must be '<font family>-<font face>'", path.string());
        }

        const auto fontFamilyName = tokens.front();
        const auto faceName = tokens.back();

        if (SUFFIXES.find(faceName) == SUFFIXES.end()) {
            EXCEPTION("Font '{}' has unknown face name '{}'", path.string(), faceName);
        }

        const auto it = assets.find(fontFamilyName);
        FontFamilyPtr fontFamily;
        if (it != assets.end()) {
            fontFamily = std::dynamic_pointer_cast<FontFamily>(it->second);
        } else {
            fontFamily = std::make_shared<FontFamily>(mod, fontFamilyName);
            add(fontFamily);
        }

        auto fontFace = std::make_shared<FontFace>(mod, path);
        fontFamily->add(faceName, fontFace);

        add(fontFace);

        return fontFace;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load font face: '{}'", path.string());
    }
}

template <> IconAtlasPtr AssetManager::load<IconAtlas>(const Manifest& mod, const Path& path) {
    try {
        const auto baseName = path.stem().string();
        const auto dir = path.parent_path();

        const auto xmlPath = dir / Path(baseName + ".xml");
        const auto pngPath = dir / Path(baseName + ".png");

        if (!Fs::exists(xmlPath)) {
            EXCEPTION("Icon atlas '{}' does not have an xml file", path.string());
        }
        if (!Fs::exists(pngPath)) {
            EXCEPTION("Icon atlas '{}' does not have a png file", path.string());
        }

        auto iconAtlas = std::make_shared<IconAtlas>(mod, baseName, pngPath);
        add(iconAtlas);

        const auto atlasSize = iconAtlas->getSize();

        Xml::Document xml(xmlPath);

        for (const auto& node : xml.getRoot()) {
            const Vector2i size = {
                node.child("width").asLong(),
                node.child("height").asLong(),
            };
            const Vector2i pos = {
                node.child("x").asLong(),
                node.child("y").asLong(),
            };
            const auto name = node.child("name").asString();

            // const auto p = Vector2{pos.x, 4096 - pos.y - size.y};

            auto icon = std::make_shared<Icon>(mod, baseName + "-" + name, *iconAtlas, pos, size);
            iconAtlas->add(name, icon);
            add(icon);
        }

        return iconAtlas;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load icon atlas: '{}'", path.string());
    }
}

template <> ModelPtr AssetManager::load<Model>(const Manifest& mod, const Path& path) {
    try {
        GltfImporter importer(path);
        const auto dir = path.parent_path();

        auto resolveTexture = [&mod, dir, this](const std::string& filename, const PbrTextureType type) {
            const auto path = dir / Path(filename);
            const auto baseName = path.stem().string();
            if (assets.find(baseName) == assets.end()) {
                try {
                    add(std::make_shared<PbrTexture>(mod, baseName, path, type));
                } catch (...) {
                    EXCEPTION_NESTED("Failed to load texture: '{}'", path.string());
                }
            }
        };

        for (const auto& material : importer.getMaterials()) {
            if (material.ambientOcclusionTexture.has_value()) {
                resolveTexture(material.ambientOcclusionTexture.value().getUri(), PbrTextureType::AmbientOcclusion);
            }
            if (material.metallicRoughnessTexture.has_value()) {
                resolveTexture(material.metallicRoughnessTexture.value().getUri(), PbrTextureType::MetallicRoughness);
            }
            if (material.emissiveTexture.has_value()) {
                resolveTexture(material.emissiveTexture.value().getUri(), PbrTextureType::Emissive);
            }
            if (material.baseColorTexture.has_value()) {
                resolveTexture(material.baseColorTexture.value().getUri(), PbrTextureType::BaseColor);
            }
            if (material.normalTexture.has_value()) {
                resolveTexture(material.normalTexture.value().getUri(), PbrTextureType::Normals);
            }
        }

        const auto baseName = path.stem().string();
        auto model = std::make_shared<Model>(mod, baseName, path);
        add(model);
        return model;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load model: '{}'", path.string());
    }
}

template <> BlockPtr AssetManager::load<Block>(const Manifest& mod, const Path& path) {
    try {
        const auto block = std::make_shared<Block>(mod, path);
        add(block);
        return block;
    } catch (...) {
        EXCEPTION_NESTED("Failed to load block: '{}'", path.string());
    }
}

void AssetManager::add(AssetPtr asset) {
    assets.insert(std::make_pair(asset->getName(), asset));
    loadQueue.push([this, asset]() {
        try {
            asset->load(*this);
        } catch (...) {
            EXCEPTION_NESTED("Failed to load asset: '{}'", asset->getName());
        }
    });
}

ImagePtr AssetManager::generateThumbnail(const Model& model) {
    static const Vector2i viewport{128, 128};
    static const Matrix4 transform{1.0f};
    const Matrix4 viewMatrix = glm::lookAt(Vector3{2.0f, 2.0f, -2.0f}, Vector3{0.0f}, Vector3{0.0f, 1.0f, 0.0f});

    thumbnailGBuffer.bind(viewport);

    renderer.setViewport(viewport);
    renderer.setProjection(30.0f);
    renderer.setCamera(viewMatrix);

    glEnable(GL_DEPTH_TEST);
    renderer.renderModel(model, transform);
    glDisable(GL_DEPTH_TEST);

    thumbnailGBuffer.unbind();

    thumbnailFbo.bind();

    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    // renderer.renderSkybox(thumbnailSkybox.texture);
    renderer.renderPbr(thumbnailGBuffer, thumbnailSkybox);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    const auto node = imageAtlas.reserve(viewport);

    /*std::unique_ptr<char[]> pixels(new char[viewport.x * viewport.y * 4]);
    glReadPixels(0, 0, viewport.x, viewport.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

    std::ofstream file(model.getName() + ".raw", std::ios::out | std::ios::binary);
    file.write(pixels.get(), viewport.x * viewport.y * 4);*/

    node->texture->bind();
    glCopyTexSubImage2D(node->texture->getTarget(), 0, node->pos.x, node->pos.y, 0, 0, viewport.x, viewport.y);

    auto image = std::make_shared<Image>(model.getMod(), model.getName() + "-thumb", imageAtlas, *node->texture,
                                         node->pos, node->size);

    add(image);
    return image;
}
