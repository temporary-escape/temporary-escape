#include "ComponentAgent.hpp"
#include "../Scene.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

BehaviorTree::Composite::~Composite() = default;

void BehaviorTree::Selector::start() {
    logger.warn("Selector start");
}

void BehaviorTree::Selector::update(const float delta, AgentState& state) {
    if (children.empty()) {
        status = Status::Failure;
        return;
    }

    if (status == Status::Running) {
        auto& child = children.at(selected);
        child->update(delta, state);

        switch (child->getStatus()) {
        case Status::Running: {
            // Nothing to do yet
            return;
        }
        case Status::Success: {
            // Child has completed, return immediately
            logger.warn("Selector success");
            status = Status::Success;
            selected = 0;
            return;
        }
        default: { // Idle or failure
            // Child has failed, we need to try the next child
            logger.warn("Selector child failure status: {}", int(child->getStatus()));
            selected += 1;
            if (selected >= children.size()) {
                logger.warn("Selector failure");
                selected = 0;
                status = Status::Failure;
            }
            return;
        }
        }
    }

    status = Status::Running;
    logger.warn("Selector choose next: {}", selected);

    // Selector will try each child in a sequence and will return immediately
    // when ANY of the children succeed.
    // If the child is running, keep it running, and go to the next child
    // if it fails.
    for (size_t i = selected; i < children.size(); i++) {
        auto& child = children.at(i);
        child->start();
        child->update(delta, state);

        // On success return immediately
        if (child->getStatus() == Status::Success) {
            status = Status::Success;
            logger.warn("Selector success");
            selected = 0;
            break;
        }

        // When the child is in a running state we need
        // to get back to it on the next update() call!
        if (child->getStatus() == Status::Running) {
            selected = i;
            status = Status::Running;
            break;
        }

        // Child has failed, try next one!
        if (child->getStatus() == Status::Failure) {
            selected = 0;
            continue;
        }
    }
}

void BehaviorTree::Sequence::start() {
    logger.warn("Sequence start");
}

void BehaviorTree::Sequence::update(const float delta, AgentState& state) {
    if (children.empty()) {
        status = Status::Failure;
        return;
    }

    // Process the current child
    status = Status::Running;
    auto& child = children.at(selected);
    if (child->getStatus() != Status::Running) {
        child->start();
    }
    child->update(delta, state);

    switch (child->getStatus()) {
    case Status::Running: {
        // Nothing to do yet
        return;
    }
    case Status::Success: {
        // Child has completed, return immediately
        selected += 1;
        if (selected >= children.size()) {
            status = Status::Success;
            selected = 0;
        }
        logger.warn("Sequence success selected: {}", selected);
        return;
    }
    default: { // Idle or failure
        // Child has failed, return immediately
        status = Status::Failure;
        logger.warn("Sequence failure child status: {}", int(child->getStatus()));
        return;
    }
    }
}

void BehaviorTree::update(const float delta, AgentState& state) {
    if (root.getStatus() == Status::Idle) {
        logger.warn("BehaviorTree root is idle");
        root.start();
    }
    root.update(delta, state);
}

class ActionPatrol : public BehaviorTree::Node {
public:
    virtual ~ActionPatrol() = default;

    void start() override {
        logger.warn("ActionPatrol start");
    }
    void update(const float delta, AgentState& state) override {
        if (!state.shipControl) {
            logger.warn("ActionPatrol ship control is none");
            status = BehaviorTree::Status::Failure;
            return;
        }

        // Choose random entity to visit
        if (status != BehaviorTree::Status::Running) {
            // const auto right = Vector3{state.transform.getAbsoluteTransform() * Vector4{1.0f, 0.0f, 0.0f, 0.0f}};
            // const auto top = Vector3{state.transform.getAbsoluteTransform() * Vector4{0.0f, 1.0f, 0.0f, 0.0f}};
            // const auto forward = glm::cross(right, top);

            /*Vector3 direction{0.0f, 0.0f, 1.0f};

            std::uniform_real_distribution<float> distDistance{1000.0f, 5000.0f};
            std::uniform_real_distribution<float> distAxis{-10000.0f, 10000.0f};
            std::uniform_real_distribution<float> distAngle{0.0f, glm::radians(60.0f)};

            const auto half = glm::radians(30.0f);

            direction = glm::rotate(direction, distAngle(state.world.rng) - half, Vector3{1.0f, 0.0f, 0.0f});
            direction = glm::rotate(direction, distAngle(state.world.rng) - half, Vector3{0.0f, 1.0f, 0.0f});

            const auto distance = distDistance(state.world.rng);

            direction *= distance;

            auto position = state.position + direction;

            // If the position is too far from the center then choose random position near the center
            if (glm::length(position) > 10000.0f) {
                position = Vector3{
                    distAxis(state.world.rng),
                    distAxis(state.world.rng),
                    distAxis(state.world.rng),
                };
            }*/

            std::uniform_real_distribution<float> distAxis{-10000.0f, 10000.0f};
            const auto position = Vector3{
                distAxis(state.world.rng),
                distAxis(state.world.rng),
                distAxis(state.world.rng),
            };

            logger.warn("ActionPatrol chosen pos: {}", position);
            state.shipControl->actionMoveTo(position);

            /*auto chosen = state.world.entities.end();
            auto maxToCheck = std::min<size_t>(state.world.entities.size(), 64);
            for (auto test = state.world.entities.begin(); test != state.world.entities.end(); test++) {
                // Discard if the entity is us
                if (test->entity == state.entity) {
                    continue;
                }

                // Discard if the distance is too short
                if (glm::distance(test->position, state.position) < 500.0f) {
                    continue;
                }

                const auto dir = glm::dot(forward, test.position - state.position);
            }*/
        }

        status = BehaviorTree::Status::Running;
        if (state.shipControl->getAction() != ShipAutopilotAction::MoveTo) {
            logger.warn("ActionPatrol move done");
            status = BehaviorTree::Status::Success;
        }
    }

private:
};

class ActionKeepIdle : public BehaviorTree::Node {
public:
    virtual ~ActionKeepIdle() = default;

    void start() override {
        logger.warn("ActionKeepIdle start");
    }
    void update(const float delta, AgentState& state) override {
        if (status != BehaviorTree::Status::Running) {
            std::uniform_real_distribution<float> dist{5.0f, 30.0f};
            time = dist(state.world.rng);
        }

        status = BehaviorTree::Status::Running;
        time -= delta;
        if (time < 0.0f) {
            status = BehaviorTree::Status::Success;
        }
    }

private:
    float time{0.0f};
};

BehaviorTree::BehaviorTree() = default;

BehaviorTree::~BehaviorTree() = default;

ComponentAgent::ComponentAgent(EntityId entity) : Component{entity}, bt{std::make_unique<BehaviorTree>()} {
    auto& root = bt->getRoot();

    { // Behavior: patrol
        auto& seq = root.addNode<BehaviorTree::Sequence>();
        seq.addNode<ActionKeepIdle>();
        seq.addNode<ActionPatrol>();
    }
}

void ComponentAgent::update(const float delta, Scene& scene, AgentWorldState& worldState,
                            ComponentTransform& transform) {
    auto* shipControl = scene.tryGetComponent<ComponentShipControl>(getEntity());
    AgentState state{
        getEntity(),
        transform.getAbsolutePosition(),
        worldState,
        transform,
        shipControl,
    };
    bt->update(delta, state);
}
