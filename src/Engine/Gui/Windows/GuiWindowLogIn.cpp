#include "GuiWindowLogIn.hpp"
#include "../../Utils/Platform.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowLogIn::GuiWindowLogIn(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, VulkanWindow& window,
                               MatchmakerClient& matchmakerClient) :
    GuiWindowModal{ctx, fontFamily, fontSize, "ONLINE SERVICES", "Connecting! Please wait..."},
    window{window},
    matchmakerClient{matchmakerClient} {

    setSize({500.0f, getSize().y});
    setCloseOnClick(false);

    addChoice("Cancel", [this]() { this->close(); });
}

void GuiWindowLogIn::update(const Vector2i& viewport) {
    GuiWindowModal::update(viewport);

    if (futureAuthMe) {
        const auto& resp = futureAuthMe.get();

        if (resp.status == 200 && matchmakerClient.hasAuthorization()) {
            // OK!
            if (authState.empty()) {
                if (onSuccess) {
                    onSuccess();
                }
            } else {
                stepLoggedIn();
            }
        } else if (resp.status == 401) {
            // Not logged in!
            stepGenerateToken();
        } else {
            setError(fmt::format("Received server error: {}", resp.error));
        }
    } else if (futureAuthState) {
        const auto& resp = futureAuthState.get();

        authState = resp.data.state;

        if (resp.status == 200) {
            // OK!
            stepProcessState();
        } else {
            setError(fmt::format("Received server error: {}", resp.error));
        }
    } else if (futureLogIn) {
        const auto& resp = futureLogIn.get();
        if (resp.status == 201 && matchmakerClient.hasAuthorization()) {
            // Logged in!
            futureAuthMe = matchmakerClient.apiAuthGetMe();
        } else if (resp.status == 204) {
            // Not yet!
        } else {
            // Error
            setError(fmt::format("Received server error: {}", resp.error));
        }
    }

    if (!futureLogIn && nextLogInTime.time_since_epoch().count() > 0) {
        tryLogIn();
    }
}

void GuiWindowLogIn::setOnSuccessCallback(OnSuccessCallback callback) {
    onSuccess = std::move(callback);
}

void GuiWindowLogIn::stepLoggedIn() {
    nextLogInTime = {};
    setText("Logged in!");

    clearChoices();
    addChoice("Done", [this]() {
        if (onSuccess) {
            onSuccess();
        }
    });
}

void GuiWindowLogIn::tryLogIn() {
    if (std::chrono::steady_clock::now() > nextLogInTime) {
        nextLogInTime = std::chrono::steady_clock::now() + std::chrono::seconds{3};
        futureLogIn = matchmakerClient.apiAuthLogIn(authState);
    }
}

void GuiWindowLogIn::stepGenerateToken() {
    setText("Not logged in to the online services. Creating token. Please wait...");
    futureAuthState = matchmakerClient.apiAuthCreateState();
}

void GuiWindowLogIn::stepProcessState() {
    setText("You must log in to use online services! Click <b><p>Open Link</b></p> to open a web browser and "
            "complete the log in process. Or click <b><p>Copy Link</b></p> to copy the link.");

    clearChoices();
    addChoice("Open Link", [this]() {
        const auto url = this->matchmakerClient.getUrlForAuthRedirect(authState);
        openWebBrowser(url);
    });
    addChoice("Copy Link", [this]() {
        const auto url = this->matchmakerClient.getUrlForAuthRedirect(authState);
        this->window.setClipboard(url);
    });
    addChoice("Cancel", [this]() { this->close(); });

    nextLogInTime = std::chrono::steady_clock::now() + std::chrono::seconds{5};
}

void GuiWindowLogIn::setError(const std::string& msg) {
    setText(msg);
    logger.error("Updated log-in status: {}", msg);
}

void GuiWindowLogIn::start() {
    nextLogInTime = {};
    authState.clear();
    futureAuthMe = matchmakerClient.apiAuthGetMe();
}
