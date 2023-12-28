#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetCombo : public GuiWidget {
public:
    using OnSelectedCallback = std::function<void(size_t, const std::string&)>;

    GuiWidgetCombo(GuiContext& ctx);

    void draw() override;

    void setOnSelected(OnSelectedCallback value);
    void addChoice(const std::string_view& value);
    void setChosen(size_t value);
    void clear();

private:
    OnSelectedCallback onClick;
    std::vector<std::string> choices;
    size_t chosen{0};
};

template <typename T>
class ENGINE_API GuiWidgetComboTyped : public GuiWidgetCombo {
public:
    using OnSelectedCallback = std::function<void(const T&)>;

    GuiWidgetComboTyped(GuiContext& ctx) : GuiWidgetCombo{ctx} {
        GuiWidgetCombo::setOnSelected([this](const size_t index, const std::string& label) {
            (void)label;

            if (index < values.size() && onSelectedCallback) {
                onSelectedCallback(values[index]);
            }
        });
    }

    void setOnSelected(OnSelectedCallback value) {
        onSelectedCallback = std::move(value);
    }

    void addChoice(const std::string_view& label, T value) {
        values.push_back(std::move(value));
        GuiWidgetCombo::addChoice(label);
    }

    void setValue(const T& value) {
        for (size_t i = 0; i < values.size(); i++) {
            if (values[i] == value) {
                GuiWidgetCombo::setChosen(i);
                return;
            }
        }
    }

    void clear() {
        GuiWidgetCombo::clear();
        values.clear();
    }

private:
    std::vector<T> values;
    OnSelectedCallback onSelectedCallback;
};
} // namespace Engine
