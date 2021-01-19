#pragma once

#include "FontFace.hpp"

namespace Scissio {
class SCISSIO_API FontFamily : public Asset {
public:
    explicit FontFamily(const Manifest& mod, std::string name);
    virtual ~FontFamily() = default;

    void load(AssetManager& assetManager) override;
    void add(std::string name, FontFacePtr face);
    const FontFacePtr& get(const std::string& name) const;

private:
    std::unordered_map<std::string, FontFacePtr> faces;
};

using FontFamilyPtr = std::shared_ptr<FontFamily>;
} // namespace Scissio
