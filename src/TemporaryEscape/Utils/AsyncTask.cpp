#include "AsyncTask.hpp"
#include "Exceptions.hpp"
#include "Log.hpp"

#define CMP "InitTask"

using namespace Engine;

AsyncTask::AsyncTask(std::function<void()> callback) : callback{std::move(callback)} {
    future = std::async([this]() {
        try {
            this->callback();
            done = true;
        } catch (...) {
            EXCEPTION_NESTED("Task failed");
        }
    });
}

void AsyncTask::resolve() {
    if (future.valid() && future.ready()) {
        future.get();
    }
}

AsyncTask::AsyncTask(AsyncTask&& other) noexcept {
    swap(other);
}

AsyncTask& AsyncTask::operator=(AsyncTask&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void AsyncTask::swap(AsyncTask& other) noexcept {
    std::swap(future, other.future);
    std::swap(callback, other.callback);
    std::swap(done, other.done);
}
