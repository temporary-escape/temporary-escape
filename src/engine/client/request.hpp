#pragma once

#include "../config.hpp"
#include "../future.hpp"
#include <functional>
#include <vector>

namespace Engine {
class AbstractRequest {
public:
    virtual ~AbstractRequest() = default;
    // virtual void complete() = 0;
};

using AbstractRequestPtr = std::shared_ptr<AbstractRequest>;

template <typename Message, typename T = typename Message::Response::ItemType> class Request : public AbstractRequest {
public:
    using Callback = std::function<void(T)>;

    explicit Request(Message msg, Callback&& callback) : msg(std::move(msg)), callback(std::move(callback)) {
    }

    ~Request() override = default;

    void complete() {
        if (!callback) {
            throw std::runtime_error("Request has no callback defined");
        }
        callback(std::move(item));
    }

    void append(T item) {
        this->item = std::move(item);
    }

    Message& getMessage() {
        return msg;
    }

private:
    Message msg;
    Callback callback;
    T item;
};

template <typename Message, typename T> class Request<Message, std::vector<T>> : public AbstractRequest {
public:
    using Callback = std::function<void(std::vector<T>)>;

    explicit Request(Message msg, Callback&& callback) : msg(std::move(msg)), callback(std::move(callback)) {
    }

    ~Request() override = default;

    void complete() {
        if (!callback) {
            throw std::runtime_error("Request has no callback defined");
        }
        callback(std::move(items));
    }

    void append(std::vector<T> chunk) {
        items.reserve(items.size() + chunk.size());
        for (auto& i : chunk) {
            items.push_back(std::move(i));
        }
    }

    Message& getMessage() {
        return msg;
    }

private:
    Message msg;
    Callback callback;
    std::vector<T> items;
};
} // namespace Engine
