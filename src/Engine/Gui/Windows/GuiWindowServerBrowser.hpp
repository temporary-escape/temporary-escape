#pragma once

#include "../../Server/MatchmakerClient.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;
class ENGINE_API GuiWindowModal;

class ENGINE_API GuiWindowServerBrowser : public GuiWindow {
public:
    using OnCreateCallback = GuiWidgetButton::OnClickCallback;
    using OnConnectCallback = std::function<void(const std::string&)>;

    explicit GuiWindowServerBrowser(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                    MatchmakerClient& matchmaker, GuiManager& guiManager);

    void fetchServers(int page);
    void setOnConnect(OnConnectCallback callback);
    void setOnCreate(OnCreateCallback callback);
    void update(const Vector2i& viewport) override;

private:
    void setMessage(const std::string& value);
    void recreateList(const MatchmakerClient::ServerPage& page);

    MatchmakerClient& matchmaker;
    GuiManager& guiManager;
    GuiWidgetGroup* group;
    GuiWidgetButton* buttonCreate;
    GuiWidgetButton* buttonConnect;
    OnConnectCallback onConnectCallback;
    std::string selected;
    Future<MatchmakerClient::ServerPageResponse> futureServerPage;
};
} // namespace Engine
