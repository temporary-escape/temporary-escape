#include "AudioContext.hpp"
#include "../Utils/Exceptions.hpp"
#include <AL/al.h>
#include <AL/alc.h>

#define CMP "AudioContext"

using namespace Engine;

struct AudioContext::Data {
    std::shared_ptr<ALCdevice> device;
    std::shared_ptr<ALCcontext> context;
};

AudioContext::AudioContext() : data(std::make_unique<Data>()) {
    if (alcIsExtensionPresent(nullptr, "ALC_enumerate_all_EXT") == AL_TRUE) {
        const auto devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
        const ALCchar *device = devices;

        while (device && *device != '\0' && strlen(device) > 0) {
            auto test = alcOpenDevice(device);
            if (test) {
                alcCloseDevice(test);
                Log::i(CMP, "Available audio device: '{}'", device);
            }

            device += strlen(device) + 1;
        }
    }

    auto device = alcOpenDevice(nullptr);
    if (!device) {
        EXCEPTION(CMP, "Failed to open audio device");
    }

    data->device = std::shared_ptr<ALCdevice>(device, [](ALCdevice* ptr) { alcCloseDevice(ptr); });

    auto context = alcCreateContext(data->device.get(), nullptr);
    if (!context) {
        EXCEPTION(CMP, "Failed to create audio context");
    }

    data->context = std::shared_ptr<ALCcontext>(context, [](ALCcontext* ptr) { alcDestroyContext(ptr); });

    if (!alcMakeContextCurrent(context)) {
        EXCEPTION(CMP, "Failed to set audio context");
    }

    const auto specifier = alcGetString(device, ALC_DEVICE_SPECIFIER);
    if (!specifier) {
        EXCEPTION(CMP, "Failed to get audio device specifier");
    }

    Log::i(CMP, "Using audio device: '{}'", specifier);
}

AudioContext::~AudioContext() = default;
