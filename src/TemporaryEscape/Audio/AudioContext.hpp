#pragma once

#include "../Library.hpp"
#include <memory>

namespace Engine {
class ENGINE_API AudioContext {
public:
    AudioContext();
    ~AudioContext();

private:
    struct Data;
    std::unique_ptr<Data> data;
};
}
