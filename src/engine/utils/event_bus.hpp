#pragma once

#include "../library.hpp"
#include "exceptions.hpp"
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine {
class ENGINE_API EventBus {
public:
    template <typename F> struct Traits;

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
    };

    EventBus() = default;
    ~EventBus() = default;
    EventBus(const EventBus& other) = delete;
    EventBus(EventBus&& other) = default;
    EventBus& operator=(const EventBus& other) = delete;
    EventBus& operator=(EventBus&& other) = default;

    template <typename Fn> void addListener(const std::string& name, Fn&& fn) {
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
    }

private:
    template <typename T, typename Fn> void addListenerForType(const std::string& name, Fn&& fn) {
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

    std::unordered_map<std::string, std::unique_ptr<EventType>> callbacks;
    std::list<std::function<void()>> queue;
};
} // namespace Engine
