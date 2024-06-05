#pragma once

#include "../../Server/Matchmaker.hpp"
#include "../../Utils/Worker.hpp"
#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiManager;
class ENGINE_API GuiWindowModal;

class ENGINE_API GuiWindowServerBrowser : public GuiWindow {
public:
    using OnCreateCallback = GuiWidgetButton::OnClickCallback;
    using OnConnectCallback = std::function<void(const std::string&)>;

    explicit GuiWindowServerBrowser(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, Matchmaker& matchmaker,
                                    GuiManager& guiManager);

    void fetchServers(int page);
    void setOnConnect(OnConnectCallback callback);
    void setOnCreate(OnCreateCallback callback);
    void update(const Vector2i& viewport) override;

private:
    void setMessage(const std::string& value);
    void recreateList(const Matchmaker::ServerPage& page);

    Matchmaker& matchmaker;
    GuiManager& guiManager;
    GuiWidgetGroup* group;
    GuiWidgetButton* buttonCreate;
    OnConnectCallback onConnectCallback;
    Future<Matchmaker::ServerPageResponse> futureServerPage;
};
} // namespace Engine
