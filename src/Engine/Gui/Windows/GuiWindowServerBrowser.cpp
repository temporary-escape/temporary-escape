#include "GuiWindowServerBrowser.hpp"
#include "../../Server/Matchmaker.hpp"
#include "../../Utils/Platform.hpp"
#include "../GuiManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowServerBrowser::GuiWindowServerBrowser(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                               Matchmaker& matchmaker, GuiManager& guiManager) :
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
        buttonCreate->setStyle(&GuiWidgetButton::infoStyle);

        auto& input = row.addWidget<GuiWidgetTextInput>();
        input.setWidth(0.0f);

        auto& search = row.addWidget<GuiWidgetButton>("Search");
        search.setWidth(100.0f, true);

        auto& clear = row.addWidget<GuiWidgetButton>("Clear");
        clear.setWidth(100.0f, true);
    }

    { // Header
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        auto& label = row.addWidget<GuiWidgetLabel>("Server list:");
        label.setWidth(0.0f);
    }

    { // Server list
        auto& row = addWidget<GuiWidgetRow>(height, 1);
        group = &row.addWidget<GuiWidgetGroup>();
        group->setScrollbar(true);
        group->setBorder(true);
    }

    setMessage("Fetching servers...");
}

void GuiWindowServerBrowser::update(const Vector2i& viewport) {
    // tasks.reset();
    // tasks.run();
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

/*void GuiWindowServerBrowser::doConnect(const std::string& id) {
    logger.info("Chosen server: {}", id);

    if (connecting) {
        return;
    }
    connecting = true;

    matchmaker.apiServersConnect(id, [this](const Matchmaker::ServerConnectResponse& res) {
        if (!res.error.empty()) {
            tasks.post([this, res]() {
                connecting = false;
                guiManager.modalDanger("Failed to connect to the server", res.error);
            });
        } else {
            tasks.post([this, res]() {
                logger.info("Received server connection address: '{}' port: {}", res.data.address, res.data.port);
                connecting = false;
                if (onConnectCallback) {
                    onConnectCallback(res.data.address, res.data.port);
                }
            });
        }
    });
}*/

void GuiWindowServerBrowser::recreateList(const Matchmaker::ServerPage& page) {
    if (page.items.empty()) {
        setMessage("No servers found!");
        return;
    }

    /*for (const auto& server : servers) {
        auto& row = group->addWidget<GuiWidgetTemplateRow>(30.0f);

        auto& connect = row.addWidget<GuiWidgetButton>("Connect");
        connect.setWidth(80.0f, true);
        connect.setOnClick([this, id = server.id]() {
            if (onConnectCallback) {
                onConnectCallback(id);
            }
        });

        auto& label = row.addWidget<GuiWidgetLabel>(server.name);
        label.setWidth(0.0f);

        auto& version = row.addWidget<GuiWidgetLabel>(server.version);
        version.setWidth(250.0f, true);
    }*/
}

void GuiWindowServerBrowser::setMessage(const std::string& value) {
    group->clearWidgets();

    auto& row = group->addWidget<GuiWidgetTemplateRow>(30.0f);
    row.addWidget<GuiWidgetLabel>(value);
}

void GuiWindowServerBrowser::fetchServers(const int page) {
    futureServerPage = matchmaker.apiServersList(page);
}
