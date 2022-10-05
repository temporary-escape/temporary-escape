#pragma once

#include "../Library.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Engine {
class Data {
public:
    // helper type for the visitor
    template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    // explicit deduction guide (not needed as of C++20)
    template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    static Data array();
    static Data map();

    using Null = std::nullptr_t;
    using Int = int64_t;
    using Float = float;
    using Bool = bool;
    using Index = size_t;
    using String = std::string;
    using Key = std::string;
    using Array = std::vector<Data>;
    using Map = std::unordered_map<Key, Data>;
    using Variant = std::variant<Null, Int, Float, Bool, String, Array, Map>;

    Data();
    Data(const Data& other);
    Data(Data&& other);
    Data(const Variant& other);
    Data(Variant&& other);

    Data& operator=(const Data& other);
    Data& operator=(Data&& other);

    Data& operator=(const Variant& other) {
        *value = other;
        return *this;
    }

    Variant& operator*() {
        return *value;
    }

    const Variant& operator*() const {
        return *value;
    }

    template <typename T> bool is() const {
        return std::holds_alternative<T>(*value);
    }

    template <typename T> T& as() {
        if (std::holds_alternative<T>(*value)) {
            return std::get<T>(*value);
        }
        throw std::runtime_error(std::string("Does not hold value of type ") + typeid(T).name());
    }

    template <typename T> const T& as() const {
        if (std::holds_alternative<T>(*value)) {
            return std::get<T>(*value);
        }
        throw std::runtime_error(std::string("Does not hold value of type ") + typeid(T).name());
    }

    Data& operator[](const Index idx) {
        if (std::holds_alternative<Array>(*value)) {
            auto& arr = std::get<Array>(*value);
            if (idx < arr.size()) {
                return arr.at(idx);
            }
            throw std::out_of_range("Index is out of range");
        }
        throw std::runtime_error("Container is not an array");
    }

    Data& operator[](const Key& key) {
        if (std::holds_alternative<Null>(*value)) {
            *value = Map{};
        }

        if (std::holds_alternative<Map>(*value)) {
            auto& map = std::get<Map>(*value);
            return map[key];
        }
        throw std::runtime_error("Container is not a map");
    }

    const Data& operator[](const Index idx) const {
        if (std::holds_alternative<Array>(*value)) {
            auto& arr = std::get<Array>(*value);
            if (arr.size() < idx) {
                return arr.at(idx);
            }
            throw std::out_of_range("Index is out of range");
        }
        throw std::runtime_error("Container is not an array");
    }

    const Data& operator[](const Key& key) const {
        if (std::holds_alternative<Null>(*value)) {
            *value = Map{};
        }

        if (std::holds_alternative<Map>(*value)) {
            auto& map = std::get<Map>(*value);
            const auto it = map.find(key);
            if (it == map.end()) {
                throw std::out_of_range("Key does not exist in the map");
            }
            return it->second;
        }
        throw std::runtime_error("Container is not a map");
    }

    void push(Variant&& other) {
        if (std::holds_alternative<Null>(*value)) {
            *value = Array{};
        }

        if (std::holds_alternative<Array>(*value)) {
            auto& arr = std::get<Array>(*value);
            arr.emplace_back(std::move(other));
            return;
        }
        throw std::runtime_error("Container is not an array");
    }

private:
    std::unique_ptr<Variant> value;
};

inline Data::Data() : value(std::make_unique<Data::Variant>()) {
    *value = nullptr;
}

inline Data::Data(const Data& other) : value(std::make_unique<Data::Variant>()) {
    std::visit([this](auto&& arg) { *value = arg; }, *other.value);
}

inline Data::Data(Data&& other) : value(std::move(other.value)) {
}

inline Data::Data(const Variant& other) : value(std::make_unique<Data::Variant>()) {
    std::visit([this](auto&& arg) { *value = arg; }, other);
}

inline Data::Data(Variant&& other) : value(std::make_unique<Data::Variant>(std::move(other))) {
}

inline Data& Data::operator=(const Data& other) {
    std::visit([this](auto&& arg) { *value = arg; }, *other.value);
    return *this;
}

inline Data& Data::operator=(Data&& other) {
    std::swap(value, other.value);
    return *this;
}

inline Data Data::array() {
    Data data{};
    data = Array{};
    return data;
}

inline Data Data::map() {
    Data data{};
    data = Map{};
    return data;
}
} // namespace Engine
