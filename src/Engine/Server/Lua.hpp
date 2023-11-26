#pragma once

#include "../Config.hpp"
#include "../Library.hpp"
#include "../Utils/EventBus.hpp"

// Forward declaration
namespace sol {
template <bool b> class basic_reference;
using reference = basic_reference<false>;
template <bool, typename> class basic_table_core;
template <bool b> using table_core = basic_table_core<b, reference>;
using table = table_core<false>;
class state;
} // namespace sol

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API Lua {
public:
    struct Data;

    explicit Lua(const Config& config, EventBus& eventBus);
    virtual ~Lua();

    void importModule(const std::string_view& name, const std::string_view& file);
    void setScene(Scene& value);
    void require(const std::string_view& name);
    void require(const std::string_view& name, const std::function<void(sol::table&)>& callback);
    sol::table& root();
    sol::state& getState();

private:
    class EventHandler;

    void setupBindings();

    const Config& config;
    std::unique_ptr<Data> data;
    std::unique_ptr<EventHandler> eventHandler;
    Scene* scene{nullptr};
};
} // namespace Engine
