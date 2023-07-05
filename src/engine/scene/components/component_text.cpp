#include "component_text.hpp"

using namespace Engine;

ComponentText::ComponentText(entt::registry& reg, entt::entity handle, std::string text, const Color4& color,
                             const float size) :
    Component{reg, handle}, text{std::move(text)}, color{color}, size{size} {
    setDirty(true);
}

void ComponentText::recalculate(FontFace& font) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

    bounds = font.getBounds(text, size);
}
