#pragma once

#include "../Utils/Exceptions.hpp"

#include "../Library.hpp"
#include "World.hpp"

#include <any>
#include <memory>
#include <string>
#include <unordered_map>

namespace Scissio {
class SCISSIO_API Store {
public:
    class Listener;

    class SCISSIO_API BasicItem {
    public:
        explicit BasicItem(Store& store) : store(store) {
        }

        virtual ~BasicItem() = 0;

        void notify();

        [[nodiscard]] size_t getKey() const {
            return reinterpret_cast<size_t>(this);
        }

    private:
        Store& store;
    };

    class SCISSIO_API Listener {
    public:
        explicit Listener(Store& store);
        virtual ~Listener();

        void notify(BasicItem& item);

        void onNotify(BasicItem& item, std::function<void(Store&)> fn) {
            callbacks.insert(std::make_pair(item.getKey(), std::move(fn)));
        }

    private:
        Store& store;
        std::unordered_map<size_t, std::function<void(Store&)>> callbacks;
    };

    template <typename T> class SCISSIO_API Item : public BasicItem {
    public:
        explicit Item(Store& store) : BasicItem(store) {
        }

        ~Item() override = default;

        [[nodiscard]] T& value() {
            return data;
        }

        [[nodiscard]] const T& value() const {
            return data;
        }

    private:
        T data;
    };

    Item<SectorDto> sector{*this};
    Item<std::vector<SystemDto>> systems{*this};
    Item<std::vector<RegionDto>> regions{*this};
    Item<std::vector<BlockDto>> blocks{*this};
    Item<std::unordered_map<std::string, ImagePtr>> thumbnails{*this};

    void notify(BasicItem& item);
    void addListener(Listener& listener);
    void removeListener(Listener& listener);

private:
    std::list<Listener*> listeners;
};
} // namespace Scissio
