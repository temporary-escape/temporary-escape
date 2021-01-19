#pragma once

#include "../Library.hpp"

#include <any>
#include <functional>
#include <mutex>
#include <queue>
#include <typeindex>

namespace Scissio {
class SCISSIO_API EventBus {
public:
    EventBus() = default;
    virtual ~EventBus() = default;

    template <typename T> void publish(T value) {
        std::lock_guard<std::mutex> guard{mutex};
        values.push_back(std::move(value));
    }

    void poll() {
        std::vector<std::any> copy;

        {
            std::lock_guard<std::mutex> guard{mutex};
            std::swap(copy, this->values);
        }

        for (const auto& value : copy) {
            const auto type = std::type_index(value.type());
            const auto it = callbacks.find(type);
            if (it != callbacks.end()) {
                it->second->consume(value);
            }
        }
    }

    template <typename T> void listen(const std::function<void(const T&)>& fn) {
        const auto type = std::type_index(typeid(T));
        auto it = callbacks.find(typeid(T));
        if (it == callbacks.end()) {
            it = callbacks.insert(std::make_pair(type, std::make_unique<TypedCallbacks<T>>())).first;
        }
        dynamic_cast<TypedCallbacks<T>*>(it->second.get())->add(fn);
    }

private:
    class Callbacks {
    public:
        virtual ~Callbacks() = default;

        virtual void consume(const std::any& value) = 0;
    };

    template <typename T> class TypedCallbacks : public Callbacks {
    public:
        virtual ~TypedCallbacks() = default;

        void consume(const std::any& value) override {
            const auto& v = std::any_cast<const T&>(value);

            for (const auto& listener : listeners) {
                listener(v);
            }
        }

        void add(const std::function<void(const T&)>& fn) {
            listeners.push_back(fn);
        }

    private:
        std::vector<std::function<void(const T&)>> listeners;
    };

    std::mutex mutex;
    std::vector<std::any> values;
    std::unordered_map<std::type_index, std::unique_ptr<Callbacks>> callbacks;
};

#define EVENT_FUNC(E, T, F) std::bind(static_cast<void (T::*)(const E&)>(&T::F), this, std::placeholders::_1)
#define ADD_EVENT_LISTENER(L, E, T, F) L.listen<E>(EVENT_FUNC(E, T, F))
} // namespace Scissio
