#include "GuiWindowServerBrowser.hpp"
#include "../../Server/Matchmaker.hpp"
#include "../GuiManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowServerBrowser::GuiWindowServerBrowser(const FontFamily& fontFamily, int fontSize, Matchmaker& matchmaker,
                                               GuiManager& guiManager) :
    GuiWindow{fontFamily, fontSize}, matchmaker{matchmaker}, guiManager{guiManager} {

    setSize({1100.0f, 600.0f});
    setTitle("Server Browser");
    setNoScrollbar(true);

    const auto height = getSize().y - 30.0 - 30.0f * 3.0f - ctx.getPadding().y * 5.0f;

    { // Top row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        auto& input = row.addWidget<GuiWidgetTextInput>();
        input.setWidth(0.0f);

        auto& search = row.addWidget<GuiWidgetButton>("Search");
        search.setWidth(100.0f, true);

        auto& clear = row.addWidget<GuiWidgetButton>("Clear");
        clear.setWidth(100.0f, true);
    }

    { // Header
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        auto& actions = row.addWidget<GuiWidgetLabel>("Action:");
        actions.setWidth(80.0f, true);

        auto& name = row.addWidget<GuiWidgetLabel>("Name:");
        name.setWidth(0.0f);

        auto& version = row.addWidget<GuiWidgetLabel>("Version:");
        version.setWidth(250.0f, true);
    }

    { // Server list
        auto& row = addWidget<GuiWidgetRow>(height, 1);
        group = &row.addWidget<GuiWidgetGroup>();
        group->setScrollbar(true);
        group->setBorder(true);
    }

    { // Bottom row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        row.addEmpty().setWidth(0.0f);

        /*auto& connect = row.addWidget<GuiWidgetButton>("Connect");
        connect.setWidth(100.0f, true);*/

        buttonClose = &row.addWidget<GuiWidgetButton>("Close");
        buttonClose->setWidth(100.0f, true);
    }
}

void GuiWindowServerBrowser::update(const Vector2i& viewport) {
    tasks.reset();
    tasks.run();
    GuiWindow::update(viewport);
}

void GuiWindowServerBrowser::setOnClose(GuiWidgetButton::OnClickCallback callback) {
    buttonClose->setOnClick(std::move(callback));
}

void GuiWindowServerBrowser::setOnConnect(OnConnectCallback callback) {
    onConnectCallback = std::move(callback);
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

void GuiWindowServerBrowser::recreateList() {
    group->clearWidgets();

    for (const auto& server : servers) {
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
    }
}

void GuiWindowServerBrowser::connect() {
    matchmaker.apiAuthLogin([this](const Matchmaker::LoginResponse& res) {
        if (!res.error.empty()) {
            tasks.post([this, res]() { guiManager.modalDanger("Failed to get servers", res.error); });
        } else {
            tasks.post([this]() {
                servers.clear();
                fetchServers(1);
            });
        }
    });
}

void GuiWindowServerBrowser::fetchServers(const int page) {
    matchmaker.apiServersGet(page, [this](const Matchmaker::ServerGetResponse res) {
        if (!res.error.empty()) {
            tasks.post([this, res]() { guiManager.modalDanger("Failed to get servers", res.error); });
        } else {
            tasks.post([this, res]() { servers.insert(servers.end(), res.data.items.begin(), res.data.items.end()); });

            if (res.data.page < res.data.pages) {
                fetchServers(res.data.page + 1);
            } else {
                tasks.post([this]() { recreateList(); });
            }
        }
    });
}
