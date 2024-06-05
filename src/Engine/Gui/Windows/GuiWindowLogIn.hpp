#pragma once

#include "../../Server/Matchmaker.hpp"
#include "GuiWindowModal.hpp"

namespace Engine {
class GuiWindowLogIn : public GuiWindowModal {
public:
    using OnSuccessCallback = std::function<void()>;

    GuiWindowLogIn(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, VulkanWindow& window,
                   Matchmaker& matchmaker);

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
    Matchmaker& matchmaker;
    OnSuccessCallback onSuccess;
    std::string authState;
    Future<Matchmaker::AuthMeResponse> futureAuthMe;
    Future<Matchmaker::AuthStateCreatedResponse> futureAuthState;
    Future<Matchmaker::AuthLogInRespose> futureLogIn;
    std::chrono::steady_clock::time_point nextLogInTime{};
};
} // namespace Engine
