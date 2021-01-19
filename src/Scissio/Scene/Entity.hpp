#pragma once

#include "ComponentSystem.hpp"
#include "Object.hpp"

#include <memory>
#include <stdexcept>

namespace Scissio {
class Scene;

class SCISSIO_API Entity : public Object {
public:
    explicit Entity(uint64_t id = 0);
    virtual ~Entity();

    uint64_t getId() const {
        return id;
    }

    template <typename T, typename... Args> std::shared_ptr<T> addComponent(Args&&... args) {
        auto ptr = std::make_shared<T>(static_cast<Object&>(*this), std::forward<Args>(args)...);
        components.push_back(ptr);
        signature |= (1 << T::Type);
        return ptr;
    }

    template <typename T> std::shared_ptr<T> getComponent() {
        if (hasComponent<T>()) {
            for (auto& component : components) {
                if (component->getType() == T::Type) {
                    return std::dynamic_pointer_cast<T>(component);
                }
            }
        }
        throw std::out_of_range("Component does not exist");
    }

    template <typename T> bool hasComponent() const {
        return signature & (1 << T::Type);
    }

    const std::vector<ComponentPtr>& getComponents() const {
        return components;
    }

    friend Scene;

private:
    uint64_t id{0};
    uint64_t signature{0};
    std::vector<ComponentPtr> components;
};

using EntityPtr = std::shared_ptr<Entity>;
} // namespace Scissio
