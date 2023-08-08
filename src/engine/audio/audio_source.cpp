#include "audio_source.hpp"
#include "../utils/exceptions.hpp"
#include "audio_buffer.hpp"
#include <AL/al.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

AudioSource::AudioSource(AudioContext& audioContext) {
    alGenSources(1, &source);
    if (const auto err = alGetError(); err != AL_NO_ERROR) {
        EXCEPTION("Failed to create audio source, error: {}", err);
    }

    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
}

AudioSource::~AudioSource() {
    if (source) {
        alDeleteSources(1, &source);
    }
}

AudioSource::AudioSource(AudioSource&& other) noexcept : source{0} {
    swap(other);
}

AudioSource& AudioSource::operator=(AudioSource&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void AudioSource::swap(AudioSource& other) {
    std::swap(source, other.source);
}

void AudioSource::bind(const AudioBuffer& buffer) {
    ALint state{0};
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        alSourceStop(source);
    }

    alSourcei(source, AL_BUFFER, static_cast<ALint>(buffer.getHandle()));
    if (const auto err = alGetError(); err != AL_NO_ERROR) {
        logger.error("Failed to bind audio buffer to source, error: {}", err);
    }

    /*auto buffers = static_cast<ALuint>(buffer.getHandle());
    alSourceQueueBuffers(source, 1, &buffers);
    if (const auto err = alGetError(); err != AL_NO_ERROR) {
        EXCEPTION("Failed to bind audio buffer to source, error: {}", err);
    }*/
}

void AudioSource::play() {
    alSourcePlay(source);
    if (const auto err = alGetError(); err != AL_NO_ERROR) {
        logger.error("Failed to play audio source, error: {}", err);
    }
}
