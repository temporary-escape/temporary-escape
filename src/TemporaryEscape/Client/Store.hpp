#pragma once

#include "../Server/Schemas.hpp"

namespace Engine {
class Store {
public:
    template <typename T> class Item {
    public:
        const T& value() const {
            return item;
        }

        T& value() {
            return item;
        }

        void markChanged() {
            for (auto& callback : callbacks) {
                callback();
            }
        }

        void onChange(std::function<void()>&& fn) {
            callbacks.template emplace_back(fn);
        }

        Item& operator=(const T& other) {
            item = other;
            return *this;
        }

        Item& operator=(T&& other) {
            std::swap(item, other);
            return *this;
        }

    private:
        T item;
        std::list<std::function<void()>> callbacks;
    };

    struct PlayerStore {
        Item<PlayerLocationData> location;
    } player;

    struct GalaxyStore {
        Item<GalaxyData> galaxy;
        Item<std::unordered_map<std::string, SystemData>> systems;
        Item<std::unordered_map<std::string, RegionData>> regions;
    } galaxy;

    struct SystemStore {
        Item<std::unordered_map<std::string, SectorPlanetData>> planets;
    } system;
};
} // namespace Engine
