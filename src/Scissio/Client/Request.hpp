#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include <functional>
#include <vector>

namespace Scissio {
/*class AbstractRequest {
public:
    virtual ~AbstractRequest() = default;
    virtual void complete() = 0;
};

using AbstractRequestPtr = std::shared_ptr<AbstractRequest>;*/

/*template <typename T> class Request : public AbstractRequest {
public:
    using type = T;

    Request(const uint64_t id) : id(id), fnThen(nullptr) {
    }

    ~Request() override = default;

    const std::vector<T>& value() const {
        return data;
    }

    void then(std::function<void(std::vector<T>)> fn) {
        fnThen = std::move(fn);
    }

    uint64_t getId() {
        return id;
    }

    void append(const std::vector<T>& data) {
        this->data.insert(this->data.end(), data.begin(), data.end());
    }

    void complete() override {
        if (fnThen) {
            fnThen(std::move(data));
        }
    }

private:
    uint64_t id;
    std::vector<T> data;
    std::function<void(std::vector<T>)> fnThen;
};

template <typename T> using RequestPtr = std::shared_ptr<Request<T>>;*/
} // namespace Scissio
