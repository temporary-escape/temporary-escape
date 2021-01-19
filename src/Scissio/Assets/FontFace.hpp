#pragma once

#include "Asset.hpp"

namespace Scissio {
class SCISSIO_API FontFace : public Asset {
public:
    explicit FontFace(const Manifest& mod, const Path& path);
    virtual ~FontFace() = default;

    void load(AssetManager& assetManager) override;
    int getHandle() const {
        return handle;
    }

private:
    Path path;
    int handle;
};

using FontFacePtr = std::shared_ptr<FontFace>;
} // namespace Scissio
