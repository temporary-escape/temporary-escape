#include "GuiBlockInfo.hpp"
#include "../Assets/AssetsManager.hpp"

using namespace Engine;

GuiBlockInfo::GuiBlockInfo(const Config& config, AssetsManager& assetsManager) {
    setNoScrollbar();
}

void GuiBlockInfo::drawLayout(Nuklear& nuklear) {
    if (!block) {
        return;
    }

    nuklear.layoutDynamic(25.0f, 1);
    nuklear.label(block->getLabel(), Nuklear::TextAlign::Left);
    nuklear.layoutDynamic(25.0f, 1);
    nuklear.label(fmt::format("Shape: {}", shapeTypeToFriendlyName(shape)), Nuklear::TextAlign::Left);

    nuklear.layoutDynamic(256.0f, 1);
    nuklear.image(block->getThumbnail(shape));
}

void GuiBlockInfo::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    Vector2 contentSize{nuklear.getSpacing().x, nuklear.getSpacing().y};

    contentSize.x += 256.0f + nuklear.getSpacing().x;
    contentSize.y += 256.0f + nuklear.getSpacing().y;

    contentSize.y += (25.0f + nuklear.getSpacing().y) * 2.0f;

    setSize(nuklear.getWindowSizeForContentRegion(contentSize));
    setPos(Vector2{viewport.x / 2, viewport.y - getSize().y - 200.0f} + offset);
}
