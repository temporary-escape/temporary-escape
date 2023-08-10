#include "../../common.hpp"
#include <engine/scene/scene.hpp>

#define TEST_TAG "[entity_tests]"

using namespace Engine;

/*class EntityRefCounter : public Entity {
public:
    EntityRefCounter(entt::registry& reg, size_t& counter) : Entity{reg}, counter{counter} {
        counter++;
    }
    ~EntityRefCounter() {
        counter--;
    }

private:
    size_t& counter;
};

TEST_CASE("Entity with child must have nested transform", TEST_TAG) {
    Scene scene{};
    auto parent = scene.createEntity();
    auto child = scene.createEntity();

    parent->addComponent<ComponentTransform>();
    child->addComponent<ComponentTransform>().setParent(parent);

    auto& parentTransform = parent->getComponent<ComponentTransform>();
    auto& childTransform = child->getComponent<ComponentTransform>();
    REQUIRE(childTransform.getAbsolutePosition() == Vector3{0.0f, 0.0f, 0.0f});

    parentTransform.translate(Vector3{5.0f, 0.0f, 0.0f});
    REQUIRE(childTransform.getAbsolutePosition() == parentTransform.getAbsolutePosition());
    REQUIRE(childTransform.getAbsolutePosition() == Vector3{5.0f, 0.0f, 0.0f});

    childTransform.translate(Vector3{0.0f, 1.0f, 0.0f});
    REQUIRE(childTransform.getAbsolutePosition() != parentTransform.getAbsolutePosition());
    REQUIRE(childTransform.getAbsolutePosition() == Vector3{5.0f, 1.0f, 0.0f});

    child->getComponent<ComponentTransform>().removeParent();
    REQUIRE(childTransform.getAbsolutePosition() == Vector3{0.0f, 1.0f, 0.0f});
}

TEST_CASE("Removing child must update the transform", TEST_TAG) {
    static size_t counter = 0;

    Scene scene{};
    auto parent = scene.createEntityOfType<EntityRefCounter>(counter);
    auto child = scene.createEntityOfType<EntityRefCounter>(counter);

    parent->addComponent<ComponentTransform>();
    child->addComponent<ComponentTransform>().setParent(parent);

    auto& parentTransform = parent->getComponent<ComponentTransform>();
    auto& childTransform = child->getComponent<ComponentTransform>();

    parentTransform.translate(Vector3{5.0f, 0.0f, 0.0f});

    REQUIRE(counter == 2);

    REQUIRE(childTransform.getAbsolutePosition() == Vector3{5.0f, 0.0f, 0.0f});
    scene.removeEntity(parent);
    parent.reset();

    REQUIRE(childTransform.getAbsolutePosition() == Vector3{0.0f, 0.0f, 0.0f});

    REQUIRE(counter == 1);
}

TEST_CASE("Adding entity with component to the scene", TEST_TAG) {
    static size_t counter = 0;

    Scene scene{};
    auto entity = scene.createEntityOfType<EntityRefCounter>(counter);
    entity->addComponent<ComponentTransform>();

    const auto& cameraSystem = scene.getView<ComponentTransform>();
    REQUIRE(cameraSystem.size_hint() == 1);

    REQUIRE(cameraSystem.size_hint() == 1);

    scene.removeEntity(entity);
    entity.reset();

    REQUIRE(cameraSystem.size_hint() == true);
    REQUIRE(counter == 0);
}*/
