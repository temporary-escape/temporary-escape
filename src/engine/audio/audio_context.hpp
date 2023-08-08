#pragma once

#include "audio_buffer.hpp"
#include "audio_source.hpp"

namespace Engine {
class ENGINE_API AudioContext {
public:
    AudioContext();
    ~AudioContext();

    AudioBuffer createBuffer(const void* src, size_t size, AudioFormat format, int frequency);
    AudioSource createSource();

private:
    struct Data;
    std::unique_ptr<Data> data;
};
} // namespace Engine
