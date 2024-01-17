#include "ComponentText.hpp"

using namespace Engine;

ComponentText::ComponentText(EntityId entity, std::string text, const Color4& color, const float size) :
    Component{entity}, dirty{true}, text{std::move(text)}, color{color}, size{size} {
}

void ComponentText::recalculate(FontFace& font) {
    if (!dirty) {
        return;
    }

    dirty = false;

    bounds = font.getBounds(text, size);
}
