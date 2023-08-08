#pragma once

#include "../audio/audio_buffer.hpp"
#include "../library.hpp"
#include "moveable_copyable.hpp"
#include "path.hpp"

namespace Engine {
class ENGINE_API OggFileReader {
public:
    explicit OggFileReader(const Path& path);
    virtual ~OggFileReader();
    NON_COPYABLE(OggFileReader);
    OggFileReader(OggFileReader&& other) noexcept;
    OggFileReader& operator=(OggFileReader&& other) noexcept;

    AudioFormat getFormat() const;
    int getFrequency() const;
    std::vector<uint8_t> readData();

private:
    struct Data;
    std::unique_ptr<Data> data;
};
} // namespace Engine
