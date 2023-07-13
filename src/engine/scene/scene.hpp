#pragma once

#include "../utils/exceptions.hpp"
#include "../vulkan/vulkan_pipeline.hpp"
#include "controller.hpp"
#include "entity.hpp"

#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine {
class ENGINE_API Skybox;

class ENGINE_API Scene : public UserInput {
public:
    using SelectedEntityCallback = std::function<void(std::optional<Entity>)>;

    explicit Scene(const Config& config, bool isServer = false);
    virtual ~Scene();

    std::tuple<Vector3, Vector3> screenToWorld(const Vector2& mousePos, float length);
    void update(float delta);

    void removeEntity(Entity& entity);
    Entity createEntity();
    Entity fromHandle(entt::entity handle);

    template <typename... Ts> auto getView() {
        return reg.view<Ts...>();
    }

    template <typename... Ts, typename Exclude> auto getView(Exclude&& exclude) {
        return reg.view<Ts...>(exclude);
    }

    void feedbackSelectedEntity(uint32_t id);
    std::optional<Entity> getSelectedEntity();

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

    ComponentCamera* getPrimaryCamera();
    const SkyboxTextures* getSkybox();

    static void bind(Lua& lua);

private:
    entt::registry reg;
    std::unordered_map<std::type_index, std::unique_ptr<Controller>> controllers;
    std::vector<UserInput*> userInputs;

    Entity primaryCamera;
    std::optional<entt::entity> selectedEntity;
};
} // namespace Engine
