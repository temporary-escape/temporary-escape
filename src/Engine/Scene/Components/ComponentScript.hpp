#pragma once

#include "../Component.hpp"

// Forward declaration
namespace sol {
template <bool b> class basic_reference;
using reference = basic_reference<false>;
template <bool, typename> class basic_table_core;
template <bool b> using table_core = basic_table_core<b, reference>;
using table = table_core<false>;
} // namespace sol

namespace Engine {
class ENGINE_API ComponentScript : public Component {
public:
    ComponentScript();
    explicit ComponentScript(EntityId entity, const sol::table& instance);
    virtual ~ComponentScript() noexcept; // NOLINT(modernize-use-override)
    NON_COPYABLE(ComponentScript);
    ComponentScript(ComponentScript&& other) noexcept;
    ComponentScript& operator=(ComponentScript&& other) noexcept;
    static constexpr auto in_place_delete = true;

    [[nodiscard]] sol::table& getInstance() const;

private:
    struct Data;
    std::unique_ptr<Data> data;
};
} // namespace Engine
