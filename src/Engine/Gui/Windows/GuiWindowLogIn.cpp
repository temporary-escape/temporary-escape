#include "GuiWindowLogIn.hpp"
#include "../../Utils/Platform.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiWindowLogIn::GuiWindowLogIn(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, VulkanWindow& window,
                               Matchmaker& matchmaker) :
    GuiWindowModal{ctx, fontFamily, fontSize, "ONLINE SERVICES", "Connecting! Please wait...", {"Cancel"}, 0},
    window{window},
    matchmaker{matchmaker} {

    setSize({500.0f, getSize().y});
    setHeaderPrimary(true);

    setOnClickCallback([this](const std::string& choice) {
        if (choice == "Done") {
            if (onSuccess) {
                onSuccess();
            }
        } else if (choice == "Cancel") {
            this->close();
        } else if (choice == "Open Link") {
            const auto url = this->matchmaker.getUrlForAuthRedirect(authState);
            openWebBrowser(url);
        } else {
            const auto url = this->matchmaker.getUrlForAuthRedirect(authState);
            this->window.setClipboard(url);
        }
    });
}

void GuiWindowLogIn::update(const Vector2i& viewport) {
    GuiWindowModal::update(viewport);

    if (futureAuthMe) {
        const auto& resp = futureAuthMe.get();

        if (resp.status == 200 && matchmaker.hasAuthorization()) {
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
        if (resp.status == 201 && matchmaker.hasAuthorization()) {
            // Logged in!
            futureAuthMe = matchmaker.apiAuthGetMe();
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
    setHeaderSuccess(true);
    setChoices({"Done"});
}

void GuiWindowLogIn::tryLogIn() {
    if (std::chrono::steady_clock::now() > nextLogInTime) {
        nextLogInTime = std::chrono::steady_clock::now() + std::chrono::seconds{3};
        futureLogIn = matchmaker.apiAuthLogIn(authState);
    }
}

void GuiWindowLogIn::stepGenerateToken() {
    setText("Not logged in to the online services. Creating token. Please wait...");
    futureAuthState = matchmaker.apiAuthCreateState();
}

void GuiWindowLogIn::stepProcessState() {
    setText("You must log in to use online services! Click <b><p>Open Link</b></p> to open a web browser and "
            "complete the log in process. Or click <b><p>Copy Link</b></p> to copy the link.");
    setChoices({
        "Open Link",
        "Copy Link",
        "Cancel",
    });

    nextLogInTime = std::chrono::steady_clock::now() + std::chrono::seconds{5};
}

void GuiWindowLogIn::setError(const std::string& msg) {
    setText(msg);
    logger.error("Updated log-in status: {}", msg);
    setHeaderDanger(true);
}

void GuiWindowLogIn::start() {
    nextLogInTime = {};
    authState.clear();
    futureAuthMe = matchmaker.apiAuthGetMe();
}
