#include "GuiWindowLoadSave.hpp"
#include "../../Database/SaveInfo.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowLoadSave::GuiWindowLoadSave(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                     GuiManager& guiManager, const Path& dir) :
    GuiWindow{ctx, fontFamily, fontSize}, dir{dir} {

    setSize({500.0f, 600.0f});
    setTitle("LOAD GAME");
    setNoScrollbar(true);
    setCloseable(true);

    const auto height = getSize().y - 30.0 - 30.0f * 2.0f - ctx.getPadding().y * 4.0f;

    { // Top row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        inputSearch = &row.addWidget<GuiWidgetTextInput>();
        inputSearch->setWidth(0.0f);

        auto& clear = row.addWidget<GuiWidgetButton>("Clear");
        clear.setWidth(100.0f, true);
        clear.setOnClick([this]() { inputSearch->setValue(""); });
    }

    { // Header
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        auto& name = row.addWidget<GuiWidgetLabel>("Save files:");
        name.setWidth(200.0f);
    }

    { // Save file list
        auto& row = addWidget<GuiWidgetRow>(height, 1);
        group = &row.addWidget<GuiWidgetGroup>();
        group->setScrollbar(true);
        group->setBorder(true);
    }
}

void GuiWindowLoadSave::setOnLoad(OnLoadCallback callback) {
    onLoad = std::move(callback);
}

void GuiWindowLoadSave::loadInfos() {
    logger.info("Loading save file infos from: '{}'", dir);

    auto infos = loadSaveInfoDir(dir);
    std::sort(infos.begin(), infos.end(), [](const SaveInfo& a, const SaveInfo& b) {
        // Sort by the time when the save file was last played
        return a.timestamp > b.timestamp;
    });

    group->clearWidgets();
    for (const auto& info : infos) {
        logger.info("Found save file: '{}'", info.path);

        auto& childRow = group->addWidget<GuiWidgetRow>(30.0f * 2.0f + 10.0f, 1);
        auto& child = childRow.addWidget<GuiWidgetGroup>();
        child.setBorder(true);
        child.setScrollbar(false);

        {
            auto& row = child.addWidget<GuiWidgetTemplateRow>(30.0f);

            if (info.compatible) {
                auto& label = row.addWidget<GuiWidgetLabel>(fmt::format("<b><p>{}</b></p>", info.path.stem().string()));
                label.setWidth(0.0f);
            } else {
                auto& label = row.addWidget<GuiWidgetLabel>(fmt::format("<b><s>{}</b></s>", info.path.stem().string()));
                label.setWidth(0.0f);
                label.setTooltip("Incompatible game version!");
            }

            if (info.compatible) {
                auto& buttonPlay = row.addWidget<GuiWidgetButton>("Play");
                buttonPlay.setWidth(80.0f, true);
                buttonPlay.setStyle(&GuiWidgetButton::successStyle);
                buttonPlay.setOnClick([this, info]() { onLoad(info.path); });

                auto& buttonDelete = row.addWidget<GuiWidgetButton>("X");
                buttonDelete.setWidth(30.0f, true);
            }
        }

        {
            auto& row = child.addWidget<GuiWidgetRow>(30.0f, 1);

            const auto text = fmt::format("<i><g>Last played: {}</g></i>", timePointToLocalString(info.timestamp));
            auto& label = row.addWidget<GuiWidgetLabel>(text);
        }
    }
}
