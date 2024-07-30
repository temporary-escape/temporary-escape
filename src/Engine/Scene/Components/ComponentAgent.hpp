#pragma once

#include "../../Assets/Image.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentShipControl;
class ENGINE_API ComponentTransform;

struct ENGINE_API AgentObservable {
    EntityId entity;
    Vector3 position;
};

struct ENGINE_API AgentWorldState {
    std::mt19937_64& rng;
    std::vector<AgentObservable> entities;
};

struct ENGINE_API AgentState {
    EntityId entity;
    Vector3 position;
    AgentWorldState& world;
    ComponentTransform& transform;
    ComponentShipControl* shipControl;
};

class ENGINE_API BehaviorTree {
public:
    NON_COPYABLE(BehaviorTree)
    MOVEABLE(BehaviorTree)

    enum class Status {
        Idle,
        Running,
        Success,
        Failure,
    };

    class Node {
    public:
        virtual ~Node() = default;

        virtual void start() = 0;
        virtual void update(float delta, AgentState& state) = 0;

        [[nodiscard]] Status getStatus() const {
            return status;
        }

    protected:
        Status status{Status::Idle};
    };

    class Composite : public Node {
    public:
        virtual ~Composite();

        template <typename T, typename... Args> T& addNode(Args&&... args) {
            auto ptr = new T(std::forward<Args>(args)...);
            children.emplace_back(static_cast<Node*>(ptr));
            return *ptr;
        }

    protected:
        using Children = std::vector<std::unique_ptr<Node>>;

        Children children;
    };

    class Selector : public Composite {
    public:
        virtual ~Selector() = default;

        void start() override;
        void update(float delta, AgentState& state) override;

    private:
        size_t selected{0};
    };

    class Sequence : public Composite {
    public:
        virtual ~Sequence() = default;

        void start() override;
        void update(float delta, AgentState& state) override;

    private:
        size_t selected{0};
    };

    BehaviorTree();
    ~BehaviorTree();
    void update(float delta, AgentState& state);

    Selector& getRoot() {
        return root;
    }

private:
    Selector root;
};

class ENGINE_API ComponentAgent : public Component {
public:
    ComponentAgent() = default;
    explicit ComponentAgent(EntityId entity);
    COMPONENT_DEFAULTS(ComponentAgent);

    void update(float delta, Scene& scene, AgentWorldState& worldState, ComponentTransform& transform);

private:
    std::unique_ptr<BehaviorTree> bt;
};
} // namespace Engine
