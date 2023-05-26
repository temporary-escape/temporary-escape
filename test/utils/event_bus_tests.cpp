#include "../common.hpp"
#include <engine/utils/event_bus.hpp>

#define TEST_TAG "[event_bus_tests]"

using namespace Engine;

/*struct EventFoo {
    std::string msg;
};

TEST_CASE("Enqueue an event and poll it", TEST_TAG) {
    EventFoo event{};
    event.msg = "Hello World!";

    std::vector<EventFoo> foos;

    EventBus bus{};
    bus.addListener("foos", [&](const EventFoo& e) { foos.push_back(e); });

    REQUIRE(foos.empty() == true);

    bus.enqueue("foos", event);

    REQUIRE(foos.empty() == true);

    bus.poll();

    REQUIRE(foos.empty() == false);
    REQUIRE(foos.size() == 1);

    bus.poll();

    REQUIRE(foos.size() == 1);
    REQUIRE(foos.front().msg == "Hello World!");
}

TEST_CASE("Register multiple handlers", TEST_TAG) {
    EventFoo event{};
    event.msg = "Hello World!";

    std::vector<EventFoo> foos;

    EventBus bus{};
    bus.addListener("foos", [&](const EventFoo& e) { foos.push_back(e); });
    bus.addListener("foos", [&](const EventFoo& e) { foos.push_back(e); });

    bus.enqueue("foos", event);
    bus.poll();

    REQUIRE(foos.size() == 2);
}

TEST_CASE("Register different event handler for same kind must fail", TEST_TAG) {
    EventBus bus{};
    bus.addListener("foos", [&](const EventFoo& e) { (void)e; });
    REQUIRE_THROWS(bus.addListener("foos", [&](const int& e) { (void)e; }));
}

TEST_CASE("Enqueue with no handlers must fail", TEST_TAG) {
    EventFoo event{};
    event.msg = "Hello World!";

    EventBus bus{};
    REQUIRE_THROWS(bus.enqueue("foos", event));
}

TEST_CASE("Enqueue bad type must fail", TEST_TAG) {
    struct EventBar {
        int a{0};
    };

    EventBar event{};
    event.a = 42;

    EventBus bus{};

    bus.addListener("foos", [&](const EventFoo& e) { (void)e; });

    REQUIRE_THROWS(bus.enqueue("foos", event));
}*/
