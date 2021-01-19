#pragma once
#include "../Graphics/Renderer.hpp"
#include "../Graphics/SkyboxRenderer.hpp"
#include "../Library.hpp"
#include "../Utils/Exceptions.hpp"
#include "Asset.hpp"
#include "Image.hpp"
#include "ImageAtlas.hpp"

#include <queue>

namespace Scissio {
class Model;
class TextureCompressor;
using AssetPtrMap = std::unordered_map<std::string, AssetPtr>;
using AssetLoadQueue = std::queue<std::function<void()>>;

class Canvas2D;

class SCISSIO_API AssetManager {
public:
    explicit AssetManager(const Config& config, Canvas2D& canvas, TextureCompressor& textureCompressor,
                          Renderer& renderer, SkyboxRenderer& skyboxRenderer);
    virtual ~AssetManager() = default;

    template <typename T> std::shared_ptr<T> findOrNull(const std::string& name) {
        const auto it = assets.find(name);
        if (it == assets.end()) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<T>(it->second);
    }

    template <typename T> std::shared_ptr<T> find(const std::string& name) {
        const auto ptr = findOrNull<T>(name);
        if (!ptr) {
            EXCEPTION("Asset '{}' does not exist", name);
        }

        return ptr;
    }

    template <typename T> std::vector<std::shared_ptr<T>> findAll() {
        std::vector<std::shared_ptr<T>> res;
        for (auto& pair : assets) {
            auto ptr = std::dynamic_pointer_cast<T>(pair.second);
            if (ptr) {
                res.push_back(ptr);
            }
        }

        return res;
    }

    template <typename T> std::shared_ptr<T> load(const Manifest& mod, const Path& path);
    AssetLoadQueue& getLoadQueue() {
        return loadQueue;
    }

    Canvas2D& getCanvas() const {
        return canvas;
    }

    TextureCompressor& getTextureCompressor() const {
        return textureCompressor;
    }

    ImagePtr generateThumbnail(const Model& model);

    static AssetManager& singleton();

private:
    static AssetManager* instance;

    void add(AssetPtr asset);

    Canvas2D& canvas;
    TextureCompressor& textureCompressor;
    Renderer& renderer;

    ImageAtlas imageAtlas;
    AssetPtrMap assets;
    AssetLoadQueue loadQueue;

    GBuffer thumbnailGBuffer;
    Skybox thumbnailSkybox;
    Framebuffer thumbnailFbo;
    Renderbuffer thumbnailFboDepth;
    Texture2D thumbnailFboColor;
};
} // namespace Scissio
