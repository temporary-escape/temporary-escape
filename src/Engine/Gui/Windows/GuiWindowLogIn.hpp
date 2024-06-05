#pragma once

#include "../../Server/MatchmakerClient.hpp"
#include "GuiWindowModal.hpp"

namespace Engine {
class GuiWindowLogIn : public GuiWindowModal {
public:
    using OnSuccessCallback = std::function<void()>;

    GuiWindowLogIn(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, VulkanWindow& window,
                   MatchmakerClient& matchmakerClient);

    void update(const Vector2i& viewport) override;
    void start();
    void setOnSuccessCallback(OnSuccessCallback callback);

private:
    void stepGenerateToken();
    void stepProcessState();
    void tryLogIn();
    void stepLoggedIn();
    void setError(const std::string& msg);

    VulkanWindow& window;
    MatchmakerClient& matchmakerClient;
    OnSuccessCallback onSuccess;
    std::string authState;
    Future<MatchmakerClient::AuthMeResponse> futureAuthMe;
    Future<MatchmakerClient::AuthStateCreatedResponse> futureAuthState;
    Future<MatchmakerClient::AuthLogInRespose> futureLogIn;
    std::chrono::steady_clock::time_point nextLogInTime{};
};
} // namespace Engine
