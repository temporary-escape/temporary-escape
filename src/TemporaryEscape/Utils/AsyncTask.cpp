#include "AsyncTask.hpp"
#include "Exceptions.hpp"
#include "Log.hpp"

#define CMP "InitTask"

using namespace Engine;

AsyncTask::AsyncTask(std::string name, std::function<void()> callback) :
    name{std::move(name)}, callback{std::move(callback)} {
}

void AsyncTask::resolve() {
    if (ready && !started) {
        started = true;
        future = std::async([this]() {
            Log::d(CMP, "Starting task: {}", name);
            try {
                callback();
                done = true;
                Log::d(CMP, "Completed task: {}", name);
            } catch (...) {
                Log::e(CMP, "Task failed: {}", name);
                EXCEPTION_NESTED("Task failed: {}", name);
            }
        });
    } else if (ready) {
        if (future.valid() && future.ready()) {
            future.get();
            if (onDone) {
                onDone();
            }
        }
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
    std::swap(name, other.name);
    std::swap(started, other.started);
    std::swap(future, other.future);
    std::swap(callback, other.callback);
    std::swap(started, other.started);
    std::swap(done, other.done);
}
