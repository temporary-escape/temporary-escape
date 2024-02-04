#include "../../Common.hpp"
#include <Engine/Utils/RingBufferPool.hpp>

using namespace Engine;

TEST_CASE("Allocate packet chunks and check iterators", "[RingBufferPool]") {
    RingBufferPool<int, 64> ring;
    REQUIRE(ring.getHead() == ring.getTail());

    auto a0 = ring.allocate();
    a0.get() = 1;
    REQUIRE(ring.getHead() != ring.getTail());

    REQUIRE(a0 == ring.getTail());
    REQUIRE(a0 != ring.getHead());
    a0.next();
    REQUIRE(a0 != ring.getTail());
    REQUIRE(a0 == ring.getHead());

    auto a1 = ring.allocate();
    a1.get() = 2;
    REQUIRE(a0 == a1);
    REQUIRE(a1 != ring.getTail());
    REQUIRE(a1 != ring.getHead());
    a1.next();
    REQUIRE(a1 != ring.getTail());
    REQUIRE(a1 == ring.getHead());

    size_t counter{1};
    auto it = ring.getTail();
    while (it != ring.getHead()) {
        REQUIRE(it.get() == counter);
        counter++;
        it.next();
    }
    REQUIRE(counter == 3);

    REQUIRE(ring.getSize() == 2);
    REQUIRE(ring.getCapacity() == 64);
}

TEST_CASE("Allocate and release iterators", "[RingBufferPool]") {
    using Ring = RingBufferPool<int, 64>;
    Ring ring;

    // Populate all items
    std::list<Ring::Iterator> iterators;
    std::unordered_set<int> ints;
    for (int i = 0; i < 1000; i++) {
        iterators.push_back(ring.allocate());
        iterators.back().get() = i;
        ints.emplace(i);
    }

    REQUIRE(ring.getSize() == 1000);
    REQUIRE(ring.getCapacity() == 1024);

    { // Validate iteration
        int counter{0};
        auto it = ring.getTail();
        while (it != ring.getHead()) {
            REQUIRE(it.get() == counter);
            counter++;
            it.next();
        }
        REQUIRE(counter == 1000);
    }

    // Erase all
    for (int i = 0; i < 1000; i++) {
        ring.pop();

        int counter{i + 1};
        auto it = ring.getTail();
        while (it != ring.getHead()) {
            REQUIRE(it.get() == counter);
            counter++;
            it.next();
        }
        REQUIRE(counter == 1000);
        REQUIRE(ring.getSize() == 1000 - i - 1);
    }

    REQUIRE(ring.getSize() == 0);
    REQUIRE(ring.getCapacity() == 64);
}
