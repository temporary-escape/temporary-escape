#pragma once

#include "AssetFontFace.hpp"

namespace Engine {
class ENGINE_API AssetFontFamily : public Asset {
public:
    explicit AssetFontFamily(const Manifest& mod, std::string name);
    virtual ~AssetFontFamily() = default;

    void load(AssetManager& assetManager) override;
    void add(std::string name, AssetFontFacePtr face);
    const AssetFontFacePtr& get(const std::string& name) const;

private:
    std::unordered_map<std::string, AssetFontFacePtr> faces;
};

using AssetFontFamilyPtr = std::shared_ptr<AssetFontFamily>;
} // namespace Engine
