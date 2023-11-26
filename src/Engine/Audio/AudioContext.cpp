#include "AudioContext.hpp"
#include "../Utils/Exceptions.hpp"
#include <AL/al.h>
#include <AL/alc.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

struct AudioContext::Data {
    std::shared_ptr<ALCdevice> device;
    std::shared_ptr<ALCcontext> context;
};

AudioContext::AudioContext() : data(std::make_unique<Data>()) {
    if (alcIsExtensionPresent(nullptr, "ALC_enumerate_all_EXT") == AL_TRUE) {
        const auto devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
        const ALCchar* device = devices;

        while (device && *device != '\0' && strlen(device) > 0) {
            auto test = alcOpenDevice(device);
            if (test) {
                alcCloseDevice(test);
                logger.info("Available audio device: '{}'", device);
            }

            device += strlen(device) + 1;
        }
    }

    auto device = alcOpenDevice(nullptr);
    if (!device) {
        EXCEPTION("Failed to open audio device");
    }

    data->device = std::shared_ptr<ALCdevice>(device, [](ALCdevice* ptr) { alcCloseDevice(ptr); });

    auto context = alcCreateContext(data->device.get(), nullptr);
    if (!context) {
        EXCEPTION("Failed to create audio context");
    }

    data->context = std::shared_ptr<ALCcontext>(context, [](ALCcontext* ptr) { alcDestroyContext(ptr); });

    if (!alcMakeContextCurrent(context)) {
        EXCEPTION("Failed to set audio context");
    }

    const auto specifier = alcGetString(device, ALC_DEVICE_SPECIFIER);
    if (!specifier) {
        EXCEPTION("Failed to get audio device specifier");
    }

    logger.info("Using audio device: '{}'", specifier);
}

AudioContext::~AudioContext() = default;

AudioBuffer Engine::AudioContext::createBuffer(const void* src, const size_t size, const AudioFormat format,
                                               const int frequency) {
    return AudioBuffer{*this, src, size, format, frequency};
}

AudioSource Engine::AudioContext::createSource() {
    return AudioSource{*this};
}
