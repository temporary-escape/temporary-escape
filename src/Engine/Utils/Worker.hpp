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

namespace Engine {
class ENGINE_API Worker {
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

        template <typename Fn> void postSafe(Fn&& fn) {
            strand->post([fn = std::forward<Fn>(fn)]() {
                try {
                    fn();
                } catch (std::exception& e) {
                    backtrace(e);
                }
            });
        }

    private:
        static void backtrace(std::exception& e);

        std::unique_ptr<asio::io_service::strand> strand;
    };

    explicit Worker(size_t num = 4);
    explicit Worker(asio::io_service& service, size_t num = 4);
    explicit Worker(std::shared_ptr<asio::io_service> service, size_t num = 4);
    ~Worker();

    Strand strand() const {
        return Strand(*service);
    }

    void run();
    void stop();

    template <typename Fn> void post(Fn&& fn) {
        service->post(std::forward<Fn>(fn));
    }

    template <typename Fn> void postSafe(Fn&& fn) {
        service->post([fn = std::forward<Fn>(fn)]() {
            try {
                fn();
            } catch (std::exception& e) {
                backtrace(e);
            }
        });
    }

private:
    static void backtrace(std::exception& e);

    void work();

    size_t start;
    std::shared_ptr<asio::io_service> service;
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cvStart;
    std::condition_variable cvStop;
    size_t counter;
    std::atomic_bool flag;
};

class ENGINE_API PeriodicWorker {
public:
    PeriodicWorker(const std::chrono::milliseconds& period);
    ~PeriodicWorker();

    template <typename Fn> void post(Fn&& fn) {
        service->post(std::forward<Fn>(fn));
    }

    template <typename Fn> void postSafe(Fn&& fn) {
        service->post([fn = std::forward<Fn>(fn)]() {
            try {
                fn();
            } catch (std::exception& e) {
                backtrace(e);
            }
        });
    }

private:
    static void backtrace(std::exception& e);

    void run();

    const std::chrono::milliseconds period;
    std::unique_ptr<asio::io_service> service;
    std::unique_ptr<asio::io_service::work> work;

    std::condition_variable cv;
    std::mutex mutex;
    bool terminate;

    std::thread thread;
};

class ENGINE_API BackgroundWorker {
public:
    explicit BackgroundWorker(size_t numTheads = 1);
    ~BackgroundWorker();

    void stop();

    template <typename Fn> void post(Fn&& fn) {
        service->post(std::forward<Fn>(fn));
    }

    template <typename Fn> void postSafe(Fn&& fn) {
        service->post([fn = std::forward<Fn>(fn)]() {
            try {
                fn();
            } catch (std::exception& e) {
                backtrace(e);
            }
        });
    }

    asio::io_service& getService() {
        return *service;
    }

    Worker::Strand strand() const {
        return Worker::Strand(*service);
    }

private:
    static void backtrace(std::exception& e);

    std::unique_ptr<asio::io_service> service;
    std::unique_ptr<asio::io_service::work> work;
    std::vector<std::thread> threads;
};

class ENGINE_API SynchronizedWorker {
public:
    void poll() {
        service.restart();
        service.run();
    }

    template <typename Fn> void post(Fn&& fn) {
        service.post(std::forward<Fn>(fn));
    }

    template <typename Fn> void postSafe(Fn&& fn) {
        service.post([fn = std::forward<Fn>(fn)]() {
            try {
                fn();
            } catch (std::exception& e) {
                backtrace(e);
            }
        });
    }

    asio::io_service& getService() {
        return service;
    }

private:
    static void backtrace(std::exception& e);

    asio::io_service service;
};
} // namespace Engine
