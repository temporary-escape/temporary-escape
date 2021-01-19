#pragma once

#include "Asset.hpp"
#include "Model.hpp"

namespace Scissio {
class Block : public Asset {
public:
    explicit Block(const Manifest& mod, const Path& path);
    virtual ~Block() = default;

    void load(AssetManager& assetManager) override;

    const std::string& getTitle() const {
        return title;
    }

    const std::string& getDescription() const {
        return description;
    }

    const ModelPtr& getModel() const {
        return model;
    }

    int getTier() const {
        return tier;
    }

    const std::string& getGroup() const {
        return group;
    }

    const ImagePtr& getThumbnail() const {
        return thumbnail;
    }

private:
    Path path;
    std::string title;
    std::string description;
    int tier;
    ModelPtr model;
    std::string group;
    ImagePtr thumbnail;
};

using BlockPtr = std::shared_ptr<Block>;
} // namespace Scissio

MSGPACK_CONVERT(Scissio::BlockPtr)
