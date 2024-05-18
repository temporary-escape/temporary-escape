#include "GuiWindowCreateSave.hpp"

#include "../../Utils/Random.hpp"
#include "../GuiManager.hpp"
#include <regex>
#include <utility>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

uint64_t convertToSeed(const std::string& text) {
    uint64_t seed{0};
    auto result = std::from_chars(text.data(), text.data() + text.size(), seed);
    if (result.ec != std::errc()) {
        return 0;
    }
    return seed;
}

std::string createNewSeed() {
    std::random_device dev{};
    std::mt19937_64 rng{dev()};
    return std::to_string(randomSeed(rng));
}

static bool isValid(const std::string& value) {
    static std::regex re{"^[a-zA-Z0-9\\s]+$"};
    std::smatch m;
    return std::regex_match(value, m, re);
}

GuiWindowCreateSave::GuiWindowCreateSave(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                         GuiManager& guiManager, Path dir) :
    GuiWindow{ctx, fontFamily, fontSize}, guiManager{guiManager}, dir{std::move(dir)} {

    setSize({500.0f, 355.0f});
    setTitle("CREATE GAME");
    setNoScrollbar(true);
    setCloseable(true);

    { // Label: name
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        row.addWidget<GuiWidgetLabel>("Save file name:");
    }

    { // Input: name
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        inputName = &row.addWidget<GuiWidgetTextInput>();

        for (auto i = 1; i < 1000; i++) {
            const auto name = fmt::format("Universe {}", i);
            const auto path = this->dir / name;
            if (!Fs::exists(path)) {
                inputName->setValue(name);
                break;
            }
        }

        inputName->setOnModify([this]() {
            const auto& val = inputName->getValue();
            if (val.size() < 3) {
                labelError->setLabel("<t>Must have at least 3 characters</t>");
                valid = false;
            } else if (val.front() == ' ') {
                labelError->setLabel("<t>Must not start with a space</t>");
                valid = false;
            } else if (val.back() == ' ') {
                labelError->setLabel("<t>Must not end with a space</t>");
                valid = false;
            } else if (!isValid(val)) {
                labelError->setLabel("<t>Contains invalid characters</t>");
                valid = false;
            } else {
                labelError->setLabel("");
                valid = true;
            }
        });
    }

    {
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        labelError = &row.addWidget<GuiWidgetLabel>("");
    }

    { // Label: seed
        auto& row = addWidget<GuiWidgetRow>(30.0f, 1);
        row.addWidget<GuiWidgetLabel>("Seed:");
    }

    { // Option: seed
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        inputSeed = &row.addWidget<GuiWidgetTextInput>();
        inputSeed->setWidth(0.0f);

        auto& reset = row.addWidget<GuiWidgetButton>("Random");
        reset.setWidth(100.0f, true);
        reset.setOnClick([this]() { inputSeed->setValue(createNewSeed()); });
        inputSeed->setValue(createNewSeed());
    }

    { // Padding
        addWidget<GuiWidgetTemplateRow>(110.0f);
    }

    { // Bottom row
        auto& row = addWidget<GuiWidgetRow>(30.0f, 3);
        row.addEmpty().setWidth(0.25f);

        buttonCreate = &row.addWidget<GuiWidgetButton>("Done");
        buttonCreate->setWidth(0.50f);
        buttonCreate->setStyle(&GuiWidgetButton::successStyle);
        buttonCreate->setOnClick([this]() {
            if (!valid) {
                return;
            }

            Form form{};
            form.seed = convertToSeed(inputSeed->getValue());

            const auto name = inputName->getValue();

            if (form.seed == 0) {
                this->guiManager.modalDanger(
                    "Error", "Seed must be a valid number!", [this](const std::string& choice) { (void)choice; });
                return;
            }

            if (name.empty()) {
                this->guiManager.modalDanger(
                    "Error", "Save file name must be valid!", [this](const std::string& choice) { (void)choice; });
                return;
            }

            form.path = this->dir / name;
            if (Fs::exists(form.path)) {
                this->guiManager.modalDanger(
                    "Error", "Save file name already exists!", [this](const std::string& choice) { (void)choice; });
                return;
            }

            this->onCreate(form);
        });

        row.addEmpty().setWidth(0.25f);
    }
}

void GuiWindowCreateSave::setOnCreate(OnCreateCallback callback) {
    onCreate = std::move(callback);
}
