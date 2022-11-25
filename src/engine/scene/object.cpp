#include "object.hpp"
#include "entity.hpp"

using namespace Engine;

Object::Object() : transform{1.0f} {
}

std::shared_ptr<Entity> Object::asEntity() {
    auto ptr = dynamic_cast<Entity*>(this);
    return ptr ? ptr->shared_from_this() : nullptr;
}
