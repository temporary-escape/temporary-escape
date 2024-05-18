#pragma once

#include "../../Server/Matchmaker.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;

class ENGINE_API GuiWindowServerBrowser : public GuiWindow {
public:
    using OnCloseCallback = GuiWidgetButton::OnClickCallback;
    using OnConnectCallback = std::function<void(const std::string&)>;

    explicit GuiWindowServerBrowser(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, Matchmaker& matchmaker,
                                    GuiManager& guiManager);

    void connect();
    void fetchServers(int page);
    void setOnConnect(OnConnectCallback callback);
    void update(const Vector2i& viewport) override;

private:
    void recreateList();

    Matchmaker& matchmaker;
    GuiManager& guiManager;
    GuiWidgetGroup* group;
    asio::io_service tasks;
    std::vector<Matchmaker::ServerModel> servers;
    OnConnectCallback onConnectCallback;
    bool connecting{false};
};
} // namespace Engine
