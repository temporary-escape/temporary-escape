#pragma once

#include <cstdlib>

namespace Engine {
template <typename T> class RefCounter : public T {
public:
    RefCounter(size_t& counter) : counter(counter) {
        counter++;
    }
    ~RefCounter() {
        counter--;
    }

private:
    size_t& counter;
};
} // namespace Engine
