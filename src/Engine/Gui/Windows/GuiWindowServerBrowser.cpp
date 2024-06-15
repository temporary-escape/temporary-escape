#include "GuiWindowServerBrowser.hpp"
#include "../../Server/MatchmakerClient.hpp"
#include "../../Utils/Platform.hpp"
#include "../GuiManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowServerBrowser::GuiWindowServerBrowser(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                               MatchmakerClient& matchmaker, GuiManager& guiManager) :
    GuiWindow{ctx, fontFamily, fontSize}, matchmaker{matchmaker}, guiManager{guiManager} {

    setSize({1100.0f, 600.0f});
    setTitle("SERVER BROWSER");
    setNoScrollbar(true);
    setCloseable(true);

    const auto height = getSize().y - 30.0 - 30.0f * 2.0f - ctx.getPadding().y * 4.0f;

    { // Top row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        buttonCreate = &row.addWidget<GuiWidgetButton>("Host Game");
        buttonCreate->setWidth(200.0f, true);
        buttonCreate->setStyle(guiStyleButtonBlue);

        auto& input = row.addWidget<GuiWidgetTextInput>();
        input.setWidth(0.0f);

        auto& search = row.addWidget<GuiWidgetButton>("Search");
        search.setWidth(100.0f, true);

        auto& clear = row.addWidget<GuiWidgetButton>("Clear");
        clear.setWidth(100.0f, true);
    }

    { // Server list
        auto& row = addWidget<GuiWidgetRow>(height, 1);
        group = &row.addWidget<GuiWidgetGroup>();
        group->setScrollbar(true);
        group->setStyle(guiStyleGroupNone);
    }

    { // Bottom row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        row.addEmpty().setWidth(0.0f);
        buttonConnect = &row.addWidget<GuiWidgetButton>("Connect");
        buttonConnect->setStyle(guiStyleButtonGrayOutline);
        buttonConnect->setWidth(200.0f, true);
        buttonConnect->setOnClick([this]() {
            if (!selected.empty() && onConnectCallback) {
                onConnectCallback(selected);
            }
        });
    }

    setMessage("Fetching servers...");
}

void GuiWindowServerBrowser::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);

    if (futureServerPage) {
        const auto& resp = futureServerPage.get();
        recreateList(resp.data);
    }
}

void GuiWindowServerBrowser::setOnConnect(OnConnectCallback callback) {
    onConnectCallback = std::move(callback);
}

void GuiWindowServerBrowser::setOnCreate(OnCreateCallback callback) {
    buttonCreate->setOnClick(std::move(callback));
}

void GuiWindowServerBrowser::recreateList(const MatchmakerClient::ServerPage& page) {
    if (page.items.empty()) {
        setMessage("No servers found!");
        return;
    }

    group->clearWidgets();

    for (const auto& server : page.items) {
        auto& groupRow = group->addWidget<GuiWidgetRow>(30.0f * 2.0f + 10.0f, 1);

        auto& child = groupRow.addWidget<GuiWidgetSelectableGroup<std::string>>(selected, server.id);
        child.setOnClick([this, serverId = server.id]() { buttonConnect->setStyle(guiStyleButtonGreenOutline); });

        { // Name
            auto& row = child.addWidget<GuiWidgetRow>(30.0f, 1);
            auto& label = row.addWidget<GuiWidgetLabel>(server.name);
        }

        { // Version
            auto& row = child.addWidget<GuiWidgetRow>(30.0f, 1);
            auto& label = row.addWidget<GuiWidgetLabel>(fmt::format("<g>Version: {} Players: 0</g>", server.version));
        }
    }
}

void GuiWindowServerBrowser::setMessage(const std::string& value) {
    group->clearWidgets();

    auto& row = group->addWidget<GuiWidgetTemplateRow>(30.0f);
    row.addWidget<GuiWidgetLabel>(value);
}

void GuiWindowServerBrowser::fetchServers(const int page) {
    buttonConnect->setStyle(guiStyleButtonGrayOutline);
    selected.clear();
    futureServerPage = matchmaker.apiServersList(page);
}
