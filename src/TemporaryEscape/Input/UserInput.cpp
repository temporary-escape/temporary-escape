#include "UserInput.hpp"

using namespace Engine;

UserInput::UserInput(const Config& config, UserInput::Handler& handler) : config{config}, handler{handler} {
    bindings.push_back(Binding{Input::CameraFreeLookLeft, Source::Key, Key::LetterA, 0});

    bindings.push_back(Binding{Input::CameraFreeLookRight, Source::Key, Key::LetterD, 0});

    bindings.push_back(Binding{Input::CameraFreeLookForward, Source::Key, Key::LetterW, 0});

    bindings.push_back(Binding{Input::CameraFreeLookBackwards, Source::Key, Key::LetterS, 0});
}

void UserInput::eventMouseMoved(const Vector2i& pos) {
}

void UserInput::eventMousePressed(const Vector2i& pos, MouseButton button) {
}

void UserInput::eventMouseReleased(const Vector2i& pos, MouseButton button) {
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
