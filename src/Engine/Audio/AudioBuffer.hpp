#pragma once

#include "../Library.hpp"
#include "../Utils/MoveableCopyable.hpp"
#include <memory>

namespace Engine {
enum class AudioFormat {
    Mono8,
    Stereo8,
    Mono16,
    Stereo16,
};

class ENGINE_API AudioContext;

class ENGINE_API AudioBuffer {
public:
    AudioBuffer() = default;
    explicit AudioBuffer(AudioContext& audioContext, const void* src, size_t size, AudioFormat format, int frequency);
    ~AudioBuffer();
    NON_COPYABLE(AudioBuffer);
    AudioBuffer(AudioBuffer&& other) noexcept;
    AudioBuffer& operator=(AudioBuffer&& other) noexcept;
    void swap(AudioBuffer& other);

    unsigned int getHandle() const {
        return buffer;
    }

private:
    void setData(const void* src, size_t size, AudioFormat format, int frequency);

    unsigned int buffer{0};
};
} // namespace Engine
