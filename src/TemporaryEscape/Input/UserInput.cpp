#include "UserInput.hpp"

using namespace Engine;

UserInput::UserInput(const Config& config, UserInput::Handler& handler) : config{config}, handler{handler} {
    bindings.push_back(Binding{Input::PointerMovement, Source::Mouse, Key::None, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookLeft, Source::Key, Key::LetterA, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookRight, Source::Key, Key::LetterD, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookForward, Source::Key, Key::LetterW, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookBackwards, Source::Key, Key::LetterS, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookUp, Source::Key, Key::SpaceBar, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookDown, Source::Key, Key::LeftControl, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookFast, Source::Key, Key::LeftShift, 0, MouseButton::None});
    bindings.push_back(Binding{Input::CameraFreeLookRotation, Source::Mouse, Key::None, 0, MouseButton::Right});
}

void UserInput::eventMouseMoved(const Vector2i& pos) {
    for (const auto& binding : bindings) {
        if (binding.source == Source::Mouse &&
            (binding.button == MouseButton::None || buttons.find(binding.button) != buttons.end())) {
            handler.eventUserInput(Event{
                true,
                binding.type,
                Vector2{pos},
            });
        }
    }
}

void UserInput::eventMousePressed(const Vector2i& pos, MouseButton button) {
    buttons.insert(button);
}

void UserInput::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    buttons.erase(button);

    for (const auto& binding : bindings) {
        if (binding.source == Source::Mouse && binding.button == button) {
            handler.eventUserInput(Event{
                false,
                binding.type,
                Vector2{pos},
            });
        }
    }
}

void UserInput::eventMouseScroll(int xscroll, int yscroll) {
}

void UserInput::eventKeyPressed(Key key, Modifiers modifiers) {
    for (const auto& binding : bindings) {
        if (binding.source == Source::Key && binding.key == key) {
            handler.eventUserInput(Event{
                true,
                binding.type,
                {0.0f, 0.0f},
            });
        }
    }
}

void UserInput::eventKeyReleased(Key key, Modifiers modifiers) {
    for (const auto& binding : bindings) {
        if (binding.source == Source::Key && binding.key == key) {
            handler.eventUserInput(Event{
                false,
                binding.type,
                {0.0f, 0.0f},
            });
        }
    }
}

void UserInput::eventWindowResized(const Vector2i& size) {
}
