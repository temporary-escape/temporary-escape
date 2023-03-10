#pragma once

namespace Engine {
enum class MouseButton {
    None,
    Left,
    Middle,
    Right,
};

enum class Key {
    None,
    LetterA,
    LetterB,
    LetterC,
    LetterD,
    LetterE,
    LetterF,
    LetterG,
    LetterH,
    LetterI,
    LetterJ,
    LetterK,
    LetterL,
    LetterM,
    LetterN,
    LetterO,
    LetterP,
    LetterQ,
    LetterR,
    LetterS,
    LetterT,
    LetterV,
    LetterW,
    LetterU,
    LetterX,
    LetterY,
    LetterZ,
    SpaceBar,
    LeftControl,
    RightControl,
    Delete,
    LeftShift,
    RightShift,
    Backspace,
    Enter,
    Tab,
};

enum class Modifier : int {
    None = 0,
    Ctrl = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
};

using Modifiers = int;

inline bool operator&(const Modifier& modifier, const Modifiers& modifiers) {
    return static_cast<int>(modifier) & modifiers;
}

inline bool operator&(const Modifiers& modifiers, const Modifier& modifier) {
    return static_cast<int>(modifier) & modifiers;
}
} // namespace Engine
