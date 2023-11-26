#pragma once

#include "../Audio/AudioBuffer.hpp"
#include "../Library.hpp"
#include "../Utils/MoveableCopyable.hpp"
#include "../Utils/Path.hpp"

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
