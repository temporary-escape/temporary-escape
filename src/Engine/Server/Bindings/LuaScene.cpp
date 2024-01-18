#include "../../Scene/Scene.hpp"
#include "Bindings.hpp"

using namespace Engine;

static void bindComponentGrid(sol::table& m) {
    auto cls = m.new_usertype<ComponentGrid>("ComponentGrid");
    cls["set_from"] = &ComponentGrid::setFrom;
}

LUA_BINDINGS(bindComponentGrid);

static void bindComponentIcon(sol::table& m) {
    auto cls = m.new_usertype<ComponentIcon>("ComponentIcon");
    cls["image"] = sol::property(&ComponentIcon::getImage, &ComponentIcon::setImage);
    cls["offset"] = sol::property(&ComponentIcon::getOffset, &ComponentIcon::setOffset);
    cls["selectable"] = sol::property(&ComponentIcon::isSelectable, &ComponentIcon::setSelectable);
    cls["environment"] = sol::property(&ComponentIcon::isEnvironment, &ComponentIcon::setEnvironment);
}

LUA_BINDINGS(bindComponentIcon);

static void bindComponentLabel(sol::table& m) {
    auto cls = m.new_usertype<ComponentLabel>("ComponentLabel");
    cls["label"] = sol::property(&ComponentLabel::getLabel, &ComponentLabel::setLabel);
}

LUA_BINDINGS(bindComponentLabel);

static void bindComponentModel(sol::table& m) {
    auto cls = m.new_usertype<ComponentModel>("ComponentModel");
    cls["model"] = sol::property(&ComponentModel::getModel, &ComponentModel::setModel);
}

LUA_BINDINGS(bindComponentModel);

static void bindComponentModelSkinned(sol::table& m) {
    auto cls = m.new_usertype<ComponentModelSkinned>("ComponentModelSkinned");
    cls["model"] = sol::property(&ComponentModelSkinned::getModel, &ComponentModelSkinned::setModel);
}

LUA_BINDINGS(bindComponentModelSkinned);

static void bindComponentScript(sol::table& m) {
    auto cls = m.new_usertype<ComponentScript>("ComponentScript");
    cls["instance"] = sol::readonly_property(&ComponentScript::getInstance);
}

LUA_BINDINGS(bindComponentScript);

static void bindComponentShipControl(sol::table& m) {
    auto cls = m.new_usertype<ComponentShipControl>("ComponentShipControl");
    cls["add_turret"] = &ComponentShipControl::addTurret;
}

LUA_BINDINGS(bindComponentShipControl);

static void bindComponentTransform(sol::table& m) {
    auto cls = m.new_usertype<ComponentTransform>("ComponentTransform");
    cls["move"] = &ComponentTransform::move;
    cls["translate"] = &ComponentTransform::translate;
    cls["scale"] = &ComponentTransform::scale;
    cls["rotate_x"] = &ComponentTransform::rotateX;
    cls["rotate_y"] = &ComponentTransform::rotateY;
    cls["rotate_z"] = &ComponentTransform::rotateZ;
    cls["rotate_by_axis"] =
        static_cast<void (ComponentTransform::*)(const Vector3&, const float)>(&ComponentTransform::rotate);
    cls["static"] = sol::property(&ComponentTransform::isStatic, &ComponentTransform::setStatic);
    using GetTransformFunc = const Matrix4& (ComponentTransform::*)() const;
    auto GetTransform = static_cast<GetTransformFunc>(&ComponentTransform::getTransform);
    cls["transform"] = sol::property(GetTransform, &ComponentTransform::setTransform);
    cls["parent"] = sol::property(&ComponentTransform::getParent, &ComponentTransform::setParent);
    cls["position"] = sol::readonly_property(&ComponentTransform::getPosition);
    cls["orientation"] = sol::readonly_property(&ComponentTransform::getOrientation);
}

LUA_BINDINGS(bindComponentTransform);

static void bindComponentTurret(sol::table& m) {
    auto cls = m.new_usertype<ComponentTurret>("ComponentTurret");
    cls["turret"] = sol::property(&ComponentTurret::getTurret, &ComponentTurret::setTurret);
    cls["target"] = sol::property(&ComponentTurret::getTarget, &ComponentTurret::setTarget);
    cls["clear_target"] = &ComponentTurret::clearTarget;
}

LUA_BINDINGS(bindComponentTurret);

static void bindScene(sol::table& m) {
    auto cls = m.new_usertype<Scene>("Scene");
    cls["create_entity"] = &Scene::createEntity;
    cls["contact_test_sphere"] = &Scene::contactTestSphere;
    cls["add_entity_template"] = &Scene::addEntityTemplate;
    cls["create_entity_template"] =
        static_cast<Entity (Scene::*)(const std::string&, const sol::table&)>(&Scene::createEntityFrom);
}

LUA_BINDINGS(bindScene);

static sol::table entityPropertyScript(Entity& self) {
    auto* component = self.tryGetComponent<ComponentScript>();
    if (component) {
        return component->getInstance();
    }
    return nullptr;
};

static void bindComponentRigidBody(sol::table& m) {
    auto cls = m.new_usertype<ComponentRigidBody>("ComponentRigidBody");
    cls["linear_velocity"] =
        sol::property(&ComponentRigidBody::getLinearVelocity, &ComponentRigidBody::setLinearVelocity);
    cls["angular_velocity"] =
        sol::property(&ComponentRigidBody::getAngularVelocity, &ComponentRigidBody::setAngularVelocity);
    cls["mass"] = sol::property(&ComponentRigidBody::getMass, &ComponentRigidBody::setMass);
    cls["scale"] = sol::property(&ComponentRigidBody::getScale, &ComponentRigidBody::setScale);
    cls["transform"] = sol::property(&ComponentRigidBody::getWorldTransform, &ComponentRigidBody::setWorldTransform);
    cls["kinematic"] = sol::property(&ComponentRigidBody::getKinematic, &ComponentRigidBody::setKinematic);
    cls["clear_forces"] = &ComponentRigidBody::clearForces;
    cls["activate"] = &ComponentRigidBody::activate;
    cls["reset_transform"] = &ComponentRigidBody::resetTransform;
}

LUA_BINDINGS(bindComponentRigidBody);

static void bindEntity(sol::table& m) {
    auto cls = m.new_usertype<Entity>("Entity");
    cls["add_component_transform"] = &Entity::addComponent<ComponentTransform>;
    cls["add_component_model"] = [](Entity& self, const ModelPtr& model) -> ComponentModel& {
        return self.addComponent<ComponentModel>(model);
    };
    cls["add_component_model_skinned"] = [](Entity& self, const ModelPtr& model) -> ComponentModelSkinned& {
        return self.addComponent<ComponentModelSkinned>(model);
    };
    cls["add_component_rigid_body"] = &Entity::addComponent<ComponentRigidBody>;
    cls["add_component_icon"] = [](Entity& self, const ImagePtr& image) -> ComponentIcon& {
        return self.addComponent<ComponentIcon>(image);
    };
    cls["add_component_label"] = [](Entity& self, const std::string& label) -> ComponentLabel& {
        return self.addComponent<ComponentLabel>(label);
    };
    cls["add_component_script"] = [](Entity& self, const sol::table& instance) -> ComponentScript& {
        return self.addComponent<ComponentScript>(instance);
    };
    cls["add_component_grid"] = [](Entity& self) -> ComponentGrid& { return self.addComponent<ComponentGrid>(); };
    cls["add_component_turret"] = [](Entity& self, const TurretPtr& turret) -> ComponentTurret& {
        return self.addComponent<ComponentTurret>(turret);
    };
    cls["add_component_ship_control"] = [](Entity& self) -> ComponentShipControl& {
        return self.addComponent<ComponentShipControl>();
    };

    cls["has_component_transform"] = &Entity::hasComponent<ComponentTransform>;
    cls["has_component_model"] = &Entity::hasComponent<ComponentModel>;
    cls["has_component_model_skinned"] = &Entity::hasComponent<ComponentModelSkinned>;
    cls["has_component_rigid_body"] = &Entity::hasComponent<ComponentRigidBody>;
    cls["has_component_icon"] = &Entity::hasComponent<ComponentIcon>;
    cls["has_component_label"] = &Entity::hasComponent<ComponentLabel>;
    cls["has_component_script"] = &Entity::hasComponent<ComponentScript>;
    cls["has_component_turret"] = &Entity::hasComponent<ComponentTurret>;
    cls["has_component_ship_control"] = &Entity::hasComponent<ComponentShipControl>;

    cls["get_component_transform"] = &Entity::tryGetComponent<ComponentTransform>;
    cls["get_component_model"] = &Entity::tryGetComponent<ComponentModel>;
    cls["get_component_model_skinned"] = &Entity::tryGetComponent<ComponentModelSkinned>;
    cls["get_component_rigid_body"] = &Entity::tryGetComponent<ComponentRigidBody>;
    cls["get_component_icon"] = &Entity::tryGetComponent<ComponentIcon>;
    cls["get_component_label"] = &Entity::tryGetComponent<ComponentLabel>;
    cls["get_component_script"] = &Entity::tryGetComponent<ComponentScript>;
    cls["get_component_grid"] = &Entity::tryGetComponent<ComponentGrid>;
    cls["get_component_turret"] = &Entity::tryGetComponent<ComponentTurret>;
    cls["get_component_ship_control"] = &Entity::tryGetComponent<ComponentShipControl>;

    cls["script"] = sol::readonly_property(&entityPropertyScript);
    cls["transform"] = sol::readonly_property(&Entity::tryGetComponent<ComponentTransform>);
    cls["model"] = sol::readonly_property(&Entity::tryGetComponent<ComponentModel>);
    cls["model_skinned"] = sol::readonly_property(&Entity::tryGetComponent<ComponentModelSkinned>);
    cls["rigid_body"] = sol::readonly_property(&Entity::tryGetComponent<ComponentRigidBody>);
    cls["icon"] = sol::readonly_property(&Entity::tryGetComponent<ComponentIcon>);
    cls["label"] = sol::readonly_property(&Entity::tryGetComponent<ComponentLabel>);
    cls["script"] = sol::readonly_property(&Entity::tryGetComponent<ComponentScript>);
    cls["grid"] = sol::readonly_property(&Entity::tryGetComponent<ComponentGrid>);
    cls["turret"] = sol::readonly_property(&Entity::tryGetComponent<ComponentTurret>);
    cls["ship_control"] = sol::readonly_property(&Entity::tryGetComponent<ComponentShipControl>);
}

LUA_BINDINGS(bindEntity);
