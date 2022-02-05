#include "../Common.hpp"
#include <TemporaryEscape/Utils/Worker.hpp>

#define TAG "[Worker]"

TEST_CASE("Run with no work", TAG) {
    Worker worker(4);
    worker.run();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_CASE("Run with one task", TAG) {
    auto completed = false;

    Worker worker(4);

    worker.post([&]() { completed = true; });
    worker.run();

    REQUIRE(completed == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_CASE("Run with multiple task", TAG) {
    std::atomic<uint64_t> completed(0);

    Worker worker(4);

    for (auto i = 0; i < 10; i++) {
        worker.post([&]() { completed.fetch_add(1); });
    }
    worker.run();

    REQUIRE(completed.load() == 10);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_CASE("Add multiple task but do not run", TAG) {
    std::atomic<uint64_t> completed(0);

    Worker worker(4);

    for (auto i = 0; i < 10; i++) {
        worker.post([&]() { completed.fetch_add(1); });
    }

    REQUIRE(completed.load() == 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
