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
using EntityRegistry = entt::registry;
using EntityId = entt::entity;
static const auto NullEntity = entt::null;

class ENGINE_API Component {
public:
    Component() = default;
    explicit Component(entt::registry& reg, entt::entity handle) : reg{&reg}, handle{handle} {
    }
    virtual ~Component() = default;

    // Used by msgpack
    void postUnpack(entt::registry& r, entt::entity h) {
        reg = &r;
        handle = h;
    }

    [[nodiscard]] bool isDirty() const {
        return dirty;
    }

    void setDirty(const bool value) {
        dirty = value;
        if (value && reg) {
            patch(*reg, handle);
        }
    }

    entt::entity getHandle() const {
        return handle;
    }
    EntityId getEntity() const {
        return handle;
    }

protected:
    virtual void patch(entt::registry& r, entt::entity h) {
        (void)r;
        (void)h;
    };

    template <typename T> T* tryGet() {
        return reg ? reg->try_get<T>(handle) : nullptr;
    }

private:
    entt::registry* reg{nullptr};
    entt::entity handle{0};

    bool dirty{false};
};

class ENGINE_API TagDisabled : public Component {
public:
    TagDisabled() = default;
    COMPONENT_DEFAULTS(TagDisabled);
};

class ENGINE_API TagStatic : public Component {
public:
    TagStatic() = default;
    COMPONENT_DEFAULTS(TagStatic);
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
