#include "entity.hpp"

using namespace Engine;

void Entity::destroy() {
    if (reg) {
        reg->destroy(handle);
    }
}
