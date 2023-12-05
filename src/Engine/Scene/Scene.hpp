#pragma once

#include "../Utils/Exceptions.hpp"
#include "../Vulkan/VulkanPipeline.hpp"
#include "Controller.hpp"
#include "Entity.hpp"

#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API Skybox;
class ENGINE_API Lua;

class ENGINE_API Scene : public UserInput {
public:
    using SelectedEntityCallback = std::function<void(std::optional<Entity>)>;

    explicit Scene(const Config& config, VoxelShapeCache* voxelShapeCache = nullptr, Lua* lua = nullptr);
    virtual ~Scene();

    std::tuple<Vector3, Vector3> screenToWorld(const Vector2& mousePos, float length) const;
    Vector2 worldToScreen(const Vector3& pos) const;
    void update(float delta);
    void recalculate(VulkanRenderer& vulkan);

    void removeEntity(Entity& entity);
    Entity createEntity();
    Entity createEntityFrom(const std::string& name, const sol::table& data);
    void addEntityTemplate(const std::string& name, const sol::table& klass);
    Entity fromHandle(entt::entity handle);

    template <typename... Ts> auto getView() {
        return reg.view<Ts...>();
    }

    template <typename... Ts, typename Exclude> auto getView(Exclude&& exclude) {
        return reg.view<Ts...>(exclude);
    }

    void feedbackSelectedEntity(uint32_t id);
    const std::optional<Entity>& getSelectedEntity() const {
        return selectedEntity;
    }

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    void setPrimaryCamera(Entity& entity) {
        primaryCamera = entity;
    }

    template <typename T, typename... Args> T& addController(Args&&... args) {
        const auto& type = std::type_index{typeid(T)};
        if (controllers.find(type) != controllers.end()) {
            EXCEPTION("Controller of type: {} already added!", typeid(T).name());
        }

        auto controller = std::make_unique<T>(reg, std::forward<Args>(args)...);
        auto ptr = dynamic_cast<T*>(controller.get());

        controllers.emplace(type, std::move(controller));

        if constexpr (std::is_base_of_v<UserInput, T>) {
            userInputs.push_back(ptr);
        }

        return *ptr;
    }

    template <typename T> T& getController() const {
        const auto& type = std::type_index{typeid(T)};
        const auto it = controllers.find(type);
        if (it == controllers.end()) {
            EXCEPTION("Controller of type: {} does not exist!", typeid(T).name());
        }
        return *dynamic_cast<T*>(it->second.get());
    }

    template <typename T> bool hasController() const {
        const auto& type = std::type_index{typeid(T)};
        if (const auto it = controllers.find(type); it != controllers.end()) {
            return true;
        }
        return false;
    }

    bool contactTestSphere(const Vector3& origin, float radius) const;

    ComponentCamera* getPrimaryCamera() const;
    const ComponentSkybox* getSkybox();
    Lua& getLua() const;

    void setSelectionEnabled(bool value) {
        selectionEnabled = value;
    }

private:
    void updateSelection();

    entt::registry reg;
    std::unordered_map<std::type_index, std::unique_ptr<Controller>> controllers;
    std::vector<UserInput*> userInputs;

    Entity primaryCamera;
    std::optional<Entity> selectedEntity{std::nullopt};
    std::optional<Entity> selectedEntityOpaque{std::nullopt};
    std::optional<Entity> selectedEntityIcon{std::nullopt};
    std::optional<Entity> selectedEntityLast{std::nullopt};
    Lua* lua{nullptr};
    std::unordered_map<std::string, std::unique_ptr<sol::table>> entityTemplates;
    bool selectionEnabled{true};
};
} // namespace Engine