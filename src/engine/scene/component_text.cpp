#include "component_text.hpp"

using namespace Engine;

void ComponentText::recalculate(FontFace& font) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    bounds = font.getBounds(text, size);
}
