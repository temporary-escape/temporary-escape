#include "bindings.hpp"
// clang-format off
#ifndef IS_WINDOWS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <utility>
#ifndef IS_WINDOWS
#pragma GCC diagnostic pop
#endif
// clang-format on

#include "../utils/event_bus.hpp"
#include "../utils/exceptions.hpp"
#include "server.hpp"

using namespace Engine;
namespace py = pybind11;
using namespace py::literals;

static auto logger = createLogger(__FILENAME__);

template <typename T> static void eventBusAddListener(const std::string& name, const py::function& fn) {
    Server::instance->getEventBus().addListener(name, [=](const T& event) { fn(event); });
}

template <typename T> static void eventBusEnqueue(const std::string& name, py::object event) {
    Server::instance->getEventBus().enqueue(name, event.cast<T>());
}

template <> void eventBusEnqueue<py::object>(const std::string& name, py::object event) {
    Server::instance->getEventBus().enqueue(name, std::move(event));
}

struct EventBusHelper {
    using AddListenerFunc = void (*)(const std::string&, const py::function&);
    using EnqueueFunc = void (*)(const std::string&, py::object);

    template <typename T> static EventBusHelper type() {
        return {
            &eventBusAddListener<T>,
            &eventBusEnqueue<T>,
        };
    }

    AddListenerFunc addListener;
    EnqueueFunc enqueue;
};

class EventBusWrapper {
public:
    static py::cpp_function listen(const std::string& name) {
        return [=](const py::function& fn) {
            logger.info("Register fn for name: {} hash: {}", name, hash(fn));
            const auto it = types.find(name);
            if (it != types.end()) {
                (*it->second.addListener)(name, fn);
            } else {
                eventBusAddListener<py::object>(name, fn);
            }

            return fn;
        };
    }

    static void remove(const py::function& fn) {
        logger.info("Remove hash: {}", hash(fn));
    }

    static void enqueue(const std::string& name, py::object obj) {
        const auto it = types.find(name);
        if (it != types.end()) {
            (*it->second.enqueue)(name, std::move(obj));
        } else {
            eventBusEnqueue<py::object>(name, std::move(obj));
        }
    }

    static const std::unordered_map<std::string, EventBusHelper> types;

private:
    std::string currentName;
};

const std::unordered_map<std::string, EventBusHelper> EventBusWrapper::types = {
    {"player_logged_in", EventBusHelper::type<EventPlayerLoggedIn>()},
    {"player_test_event", EventBusHelper::type<EventPlayerLoggedIn>()},
};

static EventBusWrapper eventBus;

PYBIND11_EMBEDDED_MODULE(engine, m) {
    m.doc() = "game engine builtin module";

    py::class_<Logger>(m, "Logger") //
        .def("info", static_cast<void (Logger::*)(const std::string&)>(&Logger::info))
        .def("debug", static_cast<void (Logger::*)(const std::string&)>(&Logger::debug))
        .def("warn", static_cast<void (Logger::*)(const std::string&)>(&Logger::warn))
        .def("error", static_cast<void (Logger::*)(const std::string&)>(&Logger::error));

    m.def("create_logger", &createLogger);

    py::class_<EventPlayerLoggedIn>(m, "EventPlayerLoggedIn") //
        .def(py::init<>())
        .def_readwrite("player_id", &EventPlayerLoggedIn::playerId);

    py::class_<EventBusWrapper>(m, "EventBus") //
        .def_static("listen", &EventBusWrapper::listen)
        .def_static("remove", &EventBusWrapper::remove)
        .def_static("enqueue", &EventBusWrapper::enqueue);

    m.attr("event_bus") = &eventBus;
}
