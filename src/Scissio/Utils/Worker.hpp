#pragma once

#define ASIO_STANDALONE

#include <asio/io_service.hpp>
#include <asio/io_service_strand.hpp>

#include "../Library.hpp"
#include "Exceptions.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace Scissio {
class PeriodicWorker {
public:
    PeriodicWorker(const std::chrono::milliseconds& period);
    ~PeriodicWorker();

    template <typename Fn> void post(Fn&& fn) {
        service->post(std::forward<Fn>(fn));
    }

private:
    void run();

    const std::chrono::milliseconds period;
    std::unique_ptr<asio::io_service> service;
    std::unique_ptr<asio::io_service::work> work;

    std::condition_variable cv;
    std::mutex mutex;
    bool terminate;

    std::thread thread;
};

class BackgroundWorker {
public:
    explicit BackgroundWorker(size_t numTheads);
    ~BackgroundWorker();

    void stop();

    template <typename Fn> void post(Fn&& fn) {
        service->post(std::forward<Fn>(fn));
    }

    std::unique_ptr<asio::io_service> service;
    std::unique_ptr<asio::io_service::work> work;
    std::vector<std::thread> threads;
};

class Worker {
public:
    class Strand {
    public:
        explicit Strand(asio::io_service& service) : strand(std::make_unique<asio::io_service::strand>(service)) {
        }
        Strand(const Strand& other) = delete;
        Strand(Strand&& other) noexcept {
            swap(other);
        }
        ~Strand() = default;
        Strand& operator=(const Strand& other) = delete;
        Strand& operator=(Strand&& other) noexcept {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }
        void swap(Strand& other) noexcept {
            std::swap(strand, other.strand);
        }

        template <typename Fn> void post(Fn&& fn) {
            strand->post(std::forward<Fn>(fn));
        }

    private:
        std::unique_ptr<asio::io_service::strand> strand;
    };

    Worker(int num = 4);
    ~Worker();

    Strand strand() const {
        return Strand(*service);
    }

    void run();

    template <typename Fn> void post(Fn&& fn) {
        service->post(std::forward<Fn>(fn));
    }

private:
    void work();

    size_t start;
    std::unique_ptr<asio::io_service> service;
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cvStart;
    std::condition_variable cvStop;
    size_t counter;
    std::atomic_bool flag;
};
} // namespace Scissio
