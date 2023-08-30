#pragma once

#include "component_transform.hpp"

namespace Engine {
class ENGINE_API ComponentRemoteHandle : public Component {
public:
    ComponentRemoteHandle() = default;
    explicit ComponentRemoteHandle(entt::registry& reg, entt::entity handle, uint64_t remoteId);
    virtual ~ComponentRemoteHandle() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentRemoteHandle);

    [[nodiscard]] uint64_t getRemoteId() const {
        return remoteId;
    }

    MSGPACK_DEFINE_ARRAY(remoteId);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    uint64_t remoteId{ComponentTransform::NullParentId};
};
} // namespace Engine
