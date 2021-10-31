#include "Store.hpp"

using namespace Scissio;

Store::Listener::Listener(Store& store) : store{store} {
    store.addListener(*this);
}

Store::Listener::~Listener() {
    store.removeListener(*this);
}

void Store::Listener::notify(BasicItem& item) {
    const auto it = callbacks.find(item.getKey());
    if (it != callbacks.end()) {
        it->second(store);
    }
}

void Store::notify(BasicItem& item) {
    for (const auto& listener : listeners) {
        listener->notify(item);
    }
}

void Store::BasicItem::notify() {
    store.notify(*this);
}

void Store::addListener(Listener& listener) {
    listeners.push_back(&listener);
}

void Store::removeListener(Listener& listener) {
    listeners.remove(&listener);
}

Store::BasicItem::~BasicItem() = default;
