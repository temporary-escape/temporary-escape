#include "ComponentSkybox.hpp"

using namespace Scissio;

ComponentSkybox::ComponentSkybox() : Component(Type) {
}

ComponentSkybox::ComponentSkybox(Object& object, uint64_t seed) : Component(Type, object), seed(seed) {
}
