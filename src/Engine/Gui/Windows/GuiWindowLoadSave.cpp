#include "GuiWindowLoadSave.hpp"
#include "../../Database/SaveInfo.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowLoadSave::GuiWindowLoadSave(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                     GuiManager& guiManager, Path dir) :
    GuiWindow{ctx, fontFamily, fontSize}, dir{std::move(dir)} {

    setSize({500.0f, 600.0f});
    setTitle("LOAD GAME");
    setNoScrollbar(true);
    setCloseable(true);

    const auto height = getSize().y - 30.0 - 30.0f * 3.0f - ctx.getPadding().y * 5.0f;

    { // Top row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        buttonCreate = &row.addWidget<GuiWidgetButton>("New Save");
        buttonCreate->setWidth(100.0f, true);
        buttonCreate->setStyle(&GuiWidgetButton::infoStyle);

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

    { // Bottom row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        buttonDelete = &row.addWidget<GuiWidgetButton>("Delete");
        buttonDelete->setWidth(100.0f, true);
        buttonDelete->setStyle(&GuiWidgetButton::dangerStyle);
        buttonDelete->setHidden(true);

        row.addEmpty().setWidth(0.0f);

        buttonPlay = &row.addWidget<GuiWidgetButton>("Play");
        buttonPlay->setWidth(150.0f, true);
        buttonPlay->setStyle(&GuiWidgetButton::successStyle);
        buttonPlay->setHidden(true);
        buttonPlay->setOnClick([this]() {
            if (!selected.empty() && selectedInfo.compatible && onLoad) {
                onLoad(selected);
            }
        });
    }
}

void GuiWindowLoadSave::setOnLoad(OnLoadCallback callback) {
    onLoad = std::move(callback);
}

void GuiWindowLoadSave::setOnCreate(OnCreateCallback callback) {
    buttonCreate->setOnClick(std::move(callback));
}

void GuiWindowLoadSave::setMode(const MultiplayerMode value) {
    mode = value;
    if (value == MultiplayerMode::LocalLan) {
        buttonPlay->setLabel("Host LAN");
        setTitle("HOST GAME");
    } else if (value == MultiplayerMode::Online) {
        buttonPlay->setLabel("Host Online");
        setTitle("HOST GAME");
    } else {
        buttonPlay->setLabel("Play");
        setTitle("LOAD GAME");
    }
}

void GuiWindowLoadSave::loadInfos() {
    selected.clear();
    buttonPlay->setHidden(true);
    buttonDelete->setHidden(true);

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
        auto& child = childRow.addWidget<GuiWidgetSelectableGroup<Path>>(selected, info.path);
        child.setOnClick([this, info]() {
            selectedInfo = info;
            buttonDelete->setHidden(false);
            if (info.compatible) {
                buttonPlay->setHidden(false);
            } else {
                buttonPlay->setHidden(true);
            }
        });

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
        }

        {
            auto& row = child.addWidget<GuiWidgetRow>(30.0f, 1);

            const auto text = fmt::format("<i><g>Last played: {}</g></i>", timePointToLocalString(info.timestamp));
            auto& label = row.addWidget<GuiWidgetLabel>(text);
        }
    }
}
