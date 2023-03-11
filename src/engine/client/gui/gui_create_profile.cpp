#include "gui_create_profile.hpp"

using namespace Engine;

GuiCreateProfile::GuiCreateProfile() {
    setSize({500.0f, 500.0f});
    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Dynamic | Nuklear::WindowFlags::Border |
             Nuklear::WindowFlags::Title);
    setTitle("Create Profile");

    form.name = "Some Player";
}

void GuiCreateProfile::drawLayout(Nuklear& nuklear) {
    nuklear.layoutDynamic(65.0f, 1);
    nuklear.text("Choose a name for your game profile. This name will be visible to you and to other players.");
    nuklear.layoutDynamic(0.0f, 1);
    nuklear.input(form.name, 64);
    nuklear.layoutDynamic(0.0f, 3);
    nuklear.layoutSkip();
    if (nuklear.button("Done")) {
        submit();
    }
    nuklear.layoutSkip();
}

void GuiCreateProfile::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - getSize().y / 2});
}

void GuiCreateProfile::submit() {
    if (onSuccess) {
        onSuccess(form);
    }
    setEnabled(false);
}
