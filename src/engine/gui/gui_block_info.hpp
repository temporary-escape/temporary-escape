#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockInfo : public GuiWindow {
public:
    explicit GuiBlockInfo(const Config& config, AssetsManager& assetsManager);
    ~GuiBlockInfo() override = default;

    void setBlock(const BlockPtr& value) {
        block = value;
    }
    void setShape(const VoxelShape::Type value) {
        shape = value;
    }
    void setOffset(const Vector2& value) {
        offset = value;
    }

private:
    void loadColors(const TexturePtr& asset);
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    BlockPtr block;
    VoxelShape::Type shape;
    Vector2 offset;
};
} // namespace Engine
