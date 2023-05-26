#pragma once

#include "../library.hpp"
#include "exceptions.hpp"
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace Engine {
using EventDataValue = std::variant<std::nullptr_t, int64_t, double, std::string>;
using EventData = std::unordered_map<std::string, EventDataValue>;

class ENGINE_API EventBus {
public:
    /*template <typename F> struct Traits;

    template <typename C, typename T> struct Traits<void (C::*)(T) const> {
        using Arg = T;
    };

    class EventType {
    public:
        virtual ~EventType() = default;

        [[nodiscard]] virtual const std::type_info& getType() const = 0;
    };

    template <typename T> class EventTypeImpl : public EventType {
    public:
        [[nodiscard]] const std::type_info& getType() const override {
            return typeid(T);
        }

        void process(const T& event) {
            for (const auto& listener : listeners) {
                listener(event);
            }
        }

        template <typename Fn> void addListener(Fn&& fn) {
            listeners.push_back(std::forward<Fn>(fn));
        }

    private:
        std::list<std::function<void(const T&)>> listeners;
    };*/

    using Callback = std::function<void(EventData)>;
    using Handle = std::list<Callback>::const_iterator;

    class Listener {
    public:
        Listener() = default;
        explicit Listener(EventBus& eventBus);
        virtual ~Listener();
        Listener(const Listener& other) = delete;
        Listener(Listener&& other) = default;
        Listener& operator=(const Listener& other) = delete;
        Listener& operator=(Listener&& other) = default;

        void reset();
        void push(const std::string& name, const EventData& data);
        Handle addHandler(std::string name, Callback fn);
        void removeHandler(const std::string& name, const Handle& handle);
        void enqueue(std::string name, EventData data);

    private:
        EventBus* eventBus{nullptr};
        std::unordered_map<std::string, std::list<Callback>> handlers;
    };

    EventBus() = default;
    ~EventBus();
    EventBus(const EventBus& other) = delete;
    EventBus(EventBus&& other) = delete;
    EventBus& operator=(const EventBus& other) = delete;
    EventBus& operator=(EventBus&& other) = delete;

    void addListener(Listener& listener);
    void removeListener(Listener& listener);
    void enqueue(std::string name, EventData data);
    void poll();

    /*template <typename Fn> void addListener(const std::string& name, Fn&& fn) {
        using T = typename Traits<decltype(&Fn::operator())>::Arg;
        addListenerForType<T>(name, std::forward<Fn>(fn));
    }

    void poll() {
        while (!queue.empty()) {
            std::function<void()> fn;
            std::swap(fn, queue.front());
            queue.pop_front();
            fn();
        }
    }

    template <typename T> void enqueue(const std::string& name, T event) {
        using Type = typename std::remove_all_extents<T>::type;

        const auto it = callbacks.find(name);
        if (it == callbacks.end()) {
            EXCEPTION("No event listener exists for event type: \'{}\'", name);
        }
        if (it->second->getType() != typeid(Type)) {
            EXCEPTION("Error casting event for event type: \'{}\'", name);
        }
        auto ptr = static_cast<EventTypeImpl<Type>*>(it->second.get());
        queue.push_back([ptr, e = std::move(event)]() { ptr->process(e); });
    }*/

private:
    struct EventDataWrapper {
        std::string name;
        EventData data;
    };
    /*template <typename T, typename Fn> void addListenerForType(const std::string& name, Fn&& fn) {
        using Type = typename std::remove_all_extents<T>::type;

        auto it = callbacks.find(name);
        if (it == callbacks.end()) {
            auto handler = std::unique_ptr<EventType>(new EventTypeImpl<Type>());
            it = callbacks.insert(std::make_pair(name, std::move(handler))).first;
        } else if (it->second->getType() != typeid(Type)) {
            EXCEPTION("Error bad cast when adding listener for event type: \'{}\'", name);
        }

        auto ptr = dynamic_cast<EventTypeImpl<Type>*>(it->second.get());
        if (!ptr) {
            EXCEPTION("Error dynamic_cast when adding listener for event type: \'{}\'", name);
        }
        ptr->addListener(std::forward<Fn>(fn));
    }

    std::unordered_map<std::string, std::unique_ptr<EventType>> callbacks;*/
    std::mutex mutex;
    std::list<EventDataWrapper> queue;
    std::list<Listener*> listeners;
};
} // namespace Engine
