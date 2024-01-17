#include "ComponentLabel.hpp"

using namespace Engine;

ComponentLabel::ComponentLabel(EntityId entity, std::string label) : Component{entity}, label{std::move(label)} {
}
