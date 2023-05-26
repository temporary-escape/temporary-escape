#include "event_bus.hpp"
#include "../server/lua.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

EventBus::Listener::Listener(EventBus& eventBus) : eventBus{&eventBus} {
    eventBus.addListener(*this);
}

EventBus::Listener::~Listener() {
    if (eventBus) {
        eventBus->removeListener(*this);
    }
}

void EventBus::Listener::reset() {
    eventBus = nullptr;
}

void EventBus::Listener::push(const std::string& name, const EventData& data) {
    const auto it = handlers.find(name);
    if (it != handlers.end()) {
        for (const auto& fn : it->second) {
            try {
                fn(data);
            } catch (std::exception& e) {
                BACKTRACE(e, "Failed to handle event: '{}'", name);
            }
        }
    }
}

void EventBus::Listener::enqueue(std::string name, EventData data) {
    if (!eventBus) {
        EXCEPTION("Failed to enqueue event: '{}', error: event bus listener is not initialized", name);
    }
    eventBus->enqueue(std::move(name), std::move(data));
}

EventBus::Handle EventBus::Listener::addHandler(std::string name, EventBus::Callback fn) {
    auto it = handlers.find(name);
    if (it == handlers.end()) {
        it = handlers.insert(std::make_pair(std::move(name), std::list<Callback>{})).first;
    }

    it->second.push_back(std::move(fn));
    return it->second.end()--;
}

void EventBus::Listener::removeHandler(const std::string& name, const Handle& handle) {
    auto it = handlers.find(name);
    if (it != handlers.end()) {
        it->second.erase(handle);
    }
}

EventBus::~EventBus() {
    std::lock_guard<std::mutex> lock{mutex};
    for (auto& listener : listeners) {
        listener->reset();
    }
}

void EventBus::enqueue(std::string name, EventData data) {
    std::lock_guard<std::mutex> lock{mutex};
    queue.push_back({std::move(name), std::move(data)});
}

void EventBus::addListener(Listener& listener) {
    std::lock_guard<std::mutex> lock{mutex};
    listeners.remove(&listener);
    listeners.push_back(&listener);
}

void EventBus::removeListener(EventBus::Listener& listener) {
    std::lock_guard<std::mutex> lock{mutex};
    listeners.remove(&listener);
}

void EventBus::poll() {
    decltype(queue) temp;

    {
        std::lock_guard<std::mutex> lock{mutex};
        std::swap(temp, queue);
    }

    if (temp.empty()) {
        return;
    }

    for (const auto& event : temp) {
        for (auto& listener : listeners) {
            listener->push(event.name, event.data);
        }
    }
}
