#include "component_remote_handle.hpp"
#include "../entity.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentRemoteHandle::ComponentRemoteHandle(entt::registry& reg, entt::entity handle, const uint64_t remoteId) :
    Component{reg, handle}, remoteId{remoteId} {
}

void ComponentRemoteHandle::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentRemoteHandle>(handle);
}
