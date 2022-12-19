#pragma once

#include "../library.hpp"
#include <atomic>
#include <memory>

namespace Engine {
class StopToken {
public:
    using State = std::atomic<bool>;

    StopToken() : state{std::make_shared<State>()} {
        state->store(false);
    }
    StopToken(const StopToken& other) = default;
    StopToken(StopToken&& other) = default;
    ~StopToken() = default;

    StopToken& operator=(const StopToken& other) = default;
    StopToken& operator=(StopToken&& other) = default;

    void stop() {
        if (state) {
            state->store(true);
        }
    }

    bool shouldStop() const {
        return !state || state->load();
    }

private:
    std::shared_ptr<State> state;
};
} // namespace Engine
