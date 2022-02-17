#include "../Common.hpp"
#include <TemporaryEscape/Scene/Scene.hpp>

#define TAG "[Entity]"

using EntityRefCounter = RefCounter<Entity>;

TEST_CASE("Entity with child must have nested transform", TAG) {
    auto parent = std::make_shared<Entity>();
    auto child = std::make_shared<Entity>();
    child->setParent(parent);

    REQUIRE(child->getAbsolutePosition() == Vector3{0.0f, 0.0f, 0.0f});

    parent->translate(Vector3{5.0f, 0.0f, 0.0f});
    REQUIRE(child->getAbsolutePosition() == parent->getAbsolutePosition());
    REQUIRE(child->getAbsolutePosition() == Vector3{5.0f, 0.0f, 0.0f});

    child->translate(Vector3{0.0f, 1.0f, 0.0f});
    REQUIRE(child->getAbsolutePosition() != parent->getAbsolutePosition());
    REQUIRE(child->getAbsolutePosition() == Vector3{5.0f, 1.0f, 0.0f});

    parent->removeChild(child);
    child.reset();
}

TEST_CASE("Removing child from parent must delete the entity", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);

    REQUIRE(counter == 2);

    parent->removeChild(child);
    child.reset();

    REQUIRE(parent->getChildren().empty());
    REQUIRE(counter == 1);
}

TEST_CASE("Setting the parent to nullptr must free the child", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);

    REQUIRE(counter == 2);

    child->setParent(nullptr);
    child.reset();

    REQUIRE(parent->getChildren().empty());
    REQUIRE(counter == 1);
}

TEST_CASE("Setting the parent to nullptr must free the parent", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);

    REQUIRE(counter == 2);

    child->setParent(nullptr);
    parent.reset();

    REQUIRE(child->getParentObject() == nullptr);
    REQUIRE(counter == 1);
}

TEST_CASE("Deleting parent must also delete children", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);
    child.reset();

    REQUIRE(counter == 2);

    parent.reset();

    REQUIRE(counter == 0);
}

TEST_CASE("Deleting children should keep it alive in parent", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);

    REQUIRE(counter == 2);

    child.reset();

    REQUIRE(counter == 2);
}

TEST_CASE("Adding parent to scene must add child as well", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);

    REQUIRE(counter == 2);

    Scene scene{};
    scene.addEntity(parent);

    REQUIRE(scene.getEntities().size() == 2);

    scene.removeEntity(parent);

    REQUIRE(scene.getEntities().empty());
    REQUIRE(parent->getChildren().empty());
    REQUIRE(child->getParentObject() == nullptr);

    child.reset();
    parent.reset();

    REQUIRE(counter == 0);
}

TEST_CASE("Removing child from scene must remove child from parent as well", TAG) {
    static size_t counter = 0;

    auto parent = std::make_shared<EntityRefCounter>(counter);
    auto child = std::make_shared<EntityRefCounter>(counter);
    child->setParent(parent);

    REQUIRE(counter == 2);

    Scene scene{};
    scene.addEntity(parent);

    REQUIRE(scene.getEntities().size() == 2);

    scene.removeEntity(child);
    child.reset();

    REQUIRE(scene.getEntities().size() == 1);
    REQUIRE(parent->getChildren().empty());
    REQUIRE(counter == 1);

    parent.reset();

    // Still referenced by scene
    REQUIRE(counter == 1);

    scene.removeEntity(scene.getEntities().front());
    REQUIRE(counter == 0);
}
