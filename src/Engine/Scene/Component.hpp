#pragma once

#include "../Math/Matrix.hpp"
#include "../Network/NetworkMessage.hpp"
#include "../Utils/Aligned.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Log.hpp"
#include "../Utils/MoveableCopyable.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include <entt/entity/registry.hpp>
#include <memory>
#include <msgpack.hpp>
#include <unordered_set>
#include <variant>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<entt::entity> {
        msgpack::object const& operator()(msgpack::object const& o, entt::entity& v) const {
            if (o.type != msgpack::type::POSITIVE_INTEGER) {
                throw std::bad_cast();
            }

            v = static_cast<entt::entity>(o.as<uint32_t>());
            return o;
        }
    };
    template <> struct pack<entt::entity> {
        template <typename Stream> packer<Stream>& operator()(msgpack::packer<Stream>& o, entt::entity const& v) const {
            o.pack(static_cast<uint32_t>(v));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack

#define COMPONENT_DEFAULTS(ClassName)                                                                                  \
    NON_COPYABLE(ClassName)                                                                                            \
    MOVEABLE(ClassName)                                                                                                \
    static constexpr auto in_place_delete = true;

namespace Engine {
class ENGINE_API Scene;
using EntityRegistry = entt::registry;
using EntityId = entt::entity;
static const auto NullEntity = entt::null;

class ENGINE_API Component {
public:
    Component() = default;
    explicit Component(EntityId entity) : entity{entity} {
    }
    virtual ~Component() = default;

    // Used by msgpack
    void postUnpack(EntityId value) {
        entity = value;
    }

    EntityId getEntity() const {
        return entity;
    }

protected:
    EntityId entity{NullEntity};
};

class ENGINE_API TagDisabled : public Component {
public:
    TagDisabled() = default;
    explicit TagDisabled(EntityId entity) : Component{entity} {
    }
    COMPONENT_DEFAULTS(TagDisabled);
};
} // namespace Engine

template <> struct fmt::formatter<Engine::EntityId> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::EntityId const& entity, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", static_cast<uint64_t>(entity));
    }
};
