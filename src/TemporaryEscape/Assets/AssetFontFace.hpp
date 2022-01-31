#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "Asset.hpp"

namespace Engine {
class ENGINE_API AssetFontFace : public Asset {
public:
    explicit AssetFontFace(const Manifest& mod, const Config& config, const Path& path);
    virtual ~AssetFontFace() = default;

    void load(AssetManager& assetManager) override;

    const Canvas2D::FontHandle& getHandle() const {
        return font;
    }

private:
    const Config& config;
    Path path;
    Canvas2D::FontHandle font;
};

using AssetFontFacePtr = std::shared_ptr<AssetFontFace>;
} // namespace Engine
