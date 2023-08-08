#include "audio_buffer.hpp"
#include "../utils/exceptions.hpp"
#include <AL/al.h>

using namespace Engine;

AudioBuffer::AudioBuffer(AudioContext& audioContext, const void* src, const size_t size, const AudioFormat format,
                         const int frequency) {
    (void)audioContext;

    alGenBuffers(1, &buffer);
    if (const auto err = alGetError(); err != AL_NO_ERROR) {
        EXCEPTION("Failed to create audio buffer, error: {}", err);
    }

    setData(src, size, format, frequency);
}

AudioBuffer::~AudioBuffer() {
    if (buffer) {
        alDeleteBuffers(1, &buffer);
    }
}

AudioBuffer::AudioBuffer(AudioBuffer&& other) noexcept : buffer{0} {
    swap(other);
}

AudioBuffer& AudioBuffer::operator=(AudioBuffer&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void AudioBuffer::swap(AudioBuffer& other) {
    std::swap(buffer, other.buffer);
}

void AudioBuffer::setData(const void* src, const size_t size, const AudioFormat format, const int frequency) {
    ALenum fmt;
    switch (format) {
    case AudioFormat::Mono8: {
        fmt = AL_FORMAT_MONO8;
        break;
    }
    case AudioFormat::Mono16: {
        fmt = AL_FORMAT_MONO16;
        break;
    }
    case AudioFormat::Stereo8: {
        fmt = AL_FORMAT_STEREO8;
        break;
    }
    case AudioFormat::Stereo16: {
        fmt = AL_FORMAT_STEREO16;
        break;
    }
    default: {
        EXCEPTION("Unsupported audio format: {}", static_cast<int>(format));
    }
    }
    alBufferData(buffer, fmt, src, size, frequency);
    if (const auto err = alGetError(); err != AL_NO_ERROR) {
        EXCEPTION("Failed to set audio buffer, error: {}", err);
    }
}
