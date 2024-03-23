#include "GuiWindowCreateSave.hpp"
#include "../../Utils/Random.hpp"
#include "../GuiManager.hpp"

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

GuiWindowCreateSave::GuiWindowCreateSave(const FontFamily& fontFamily, int fontSize, GuiManager& guiManager,
                                         const Path& dir) :
    GuiWindow{fontFamily, fontSize}, guiManager{guiManager}, dir{dir} {

    setSize({500.0f, 350.0f});
    setTitle("Create Game");
    setNoScrollbar(true);

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
        addWidget<GuiWidgetTemplateRow>(145.0f);
    }

    { // Bottom row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        buttonCreate = &row.addWidget<GuiWidgetButton>("Done");
        buttonCreate->setWidth(100.0f, true);
        buttonCreate->setStyle(&GuiWidgetButton::successStyle);
        buttonCreate->setOnClick([this]() {
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

        row.addEmpty().setWidth(0.0f);

        buttonClose = &row.addWidget<GuiWidgetButton>("Close");
        buttonClose->setWidth(100.0f, true);
    }
}

void GuiWindowCreateSave::setOnCreate(OnCreateCallback callback) {
    onCreate = std::move(callback);
}

void GuiWindowCreateSave::setOnClose(OnCloseCallback callback) {
    buttonClose->setOnClick(std::move(callback));
}
