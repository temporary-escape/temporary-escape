#pragma once

#include "../Library.hpp"
#include "../Utils/MoveableCopyable.hpp"
#include <memory>

namespace Engine {
class ENGINE_API AudioContext;
class ENGINE_API AudioBuffer;

class ENGINE_API AudioSource {
public:
    AudioSource() = default;
    explicit AudioSource(AudioContext& audioContext);
    ~AudioSource();
    NON_COPYABLE(AudioSource);
    AudioSource(AudioSource&& other) noexcept;
    AudioSource& operator=(AudioSource&& other) noexcept;
    void swap(AudioSource& other);

    void bind(const AudioBuffer& buffer);
    void play();

private:
    unsigned int source{0};
};
} // namespace Engine
