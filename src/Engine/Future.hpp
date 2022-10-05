#pragma once
#include <functional>
#include <future>

namespace Engine {
template <typename T> class Future {
public:
    Future() = default;

    Future(std::future<T>&& other) noexcept : f(std::move(other)) {
    }

    Future(const Future<T>& other) = delete;

    Future(Future<T>&& other) noexcept : f(std::move(other.f)) {
    }

    bool valid() const {
        return f.valid();
    }

    bool ready() const {
        return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void wait() const {
        return f.wait();
    }

    template <class Rep, class Period>
    std::future_status waitFor(const std::chrono::duration<Rep, Period>& duration) const {
        return f.template wait_for<Rep, Period>(duration);
    }

    T get() {
        return f.get();
    }

    template <class Rep, class Period> T get(const std::chrono::duration<Rep, Period>& duration) {
        if (waitFor(duration) == std::future_status::timeout) {
            throw std::runtime_error("timeout while waiting for promise");
        }
        return get();
    }

    operator bool() const {
        return ready();
    }

    void swap(Future<T>& other) noexcept {
        std::swap(f, other.f);
    }

    Future<T>& operator=(const Future<T>& other) = delete;

    Future<T>& operator=(Future<T>&& other) noexcept {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }

private:
    std::future<T> f;
};

template <typename T> class Promise {
public:
    Promise() = default;

    Promise(std::promise<T>&& other) noexcept : p(std::move(other)) {
    }

    Promise(Promise<T>&& other) noexcept : p(std::move(other.p)) {
    }

    Promise(const Promise<T>& other) = delete;

    Future<T> future() {
        return Future<T>(p.get_future());
    }

    void swap(Promise<T>& other) noexcept {
        std::swap(p, other.p);
    }

    Promise<T>& operator=(Promise<T>&& other) noexcept {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }

    Promise<T>& operator=(const Promise<T>& other) = delete;

    void resolve(const T& value) {
        p.set_value(value);
    }
    void resolve(T& value) {
        p.set_value(value);
    }
    void resolve(T&& value) {
        p.set_value(std::move(value));
    }

    template <typename E, typename... Args> void reject(Args&&... args) {
        try {
            throw E(std::forward<Args>(args)...);
        } catch (...) {
            p.set_exception(std::current_exception());
        }
    }

    void reject(std::exception_ptr& eptr) {
        p.set_exception(eptr);
    }

private:
    std::promise<T> p;
};

template <> class Promise<void> {
public:
    Promise() = default;

    Promise(std::promise<void>&& other) noexcept : p(std::move(other)) {
    }

    Promise(Promise<void>&& other) noexcept : p(std::move(other.p)) {
    }

    Promise(const Promise<void>& other) = delete;

    Future<void> future() {
        return Future<void>(p.get_future());
    }

    void swap(Promise<void>& other) noexcept {
        std::swap(p, other.p);
    }

    Promise<void>& operator=(Promise<void>&& other) noexcept {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }

    Promise<void>& operator=(const Promise<void>& other) = delete;

    void resolve() {
        p.set_value();
    }

    template <typename E, typename... Args> void reject(Args&&... args) {
        try {
            throw E(std::forward<Args>(args)...);
        } catch (...) {
            p.set_exception(std::current_exception());
        }
    }

    void reject(std::exception_ptr& eptr) {
        p.set_exception(eptr);
    }

private:
    std::promise<void> p;
};

template <typename T> using PromisePtr = std::shared_ptr<Promise<T>>;

template <typename T> Future<T> asyncFuture(const std::function<T(void)>& fn) {
    return Future<T>(std::async(fn));
}

template <typename Func, typename Ret = typename std::invoke_result<Func>::type> Future<Ret> async(Func&& fn) {
    return Future<Ret>(std::async(fn));
}
} // namespace Engine
