#include "ogg_file_reader.hpp"
#include "../utils/exceptions.hpp"
#include <fstream>
#include <vorbis/vorbisfile.h>

using namespace Engine;

// Implemented based on: https://stackoverflow.com/a/52121855/3760426
static size_t read(void* buffer, const size_t elementSize, const size_t elementCount, void* dataSource) {
    assert(elementSize == 1);

    std::ifstream& stream = *static_cast<std::ifstream*>(dataSource);
    stream.read(static_cast<char*>(buffer), elementCount);
    const std::streamsize bytesRead = stream.gcount();
    stream.clear(); // In case we read past EOF
    return static_cast<size_t>(bytesRead);
}

static int seek(void* dataSource, const ogg_int64_t offset, const int origin) {
    static const std::vector<std::ios_base::seekdir> seekDirections{
        std::ios_base::beg, std::ios_base::cur, std::ios_base::end};

    std::ifstream& stream = *static_cast<std::ifstream*>(dataSource);
    stream.seekg(offset, seekDirections.at(origin));
    stream.clear(); // In case we seeked to EOF
    return 0;
}

static long tell(void* dataSource) {
    std::ifstream& stream = *static_cast<std::ifstream*>(dataSource);
    const auto position = stream.tellg();
    assert(position >= 0);
    return static_cast<long>(position);
}

struct OggFileReader::Data {
    OggVorbis_File file;
    ov_callbacks callbacks;
    std::fstream input;
    vorbis_info* info{nullptr};
};

OggFileReader::OggFileReader(const Path& path) : data{std::make_unique<Data>()} {
    data->input = std::fstream{path, std::ios::binary | std::ios::in};
    if (!data->input) {
        EXCEPTION("Failed to open OGG file: '{}'", path);
    }

    data->callbacks = {read, seek, nullptr, tell};
    if (const auto res = ov_open_callbacks(&data->input, &data->file, nullptr, 0, data->callbacks); res < 0) {
        EXCEPTION("Failed to read OGG file: '{}'", path);
    }

    data->info = ov_info(&data->file, -1);

    if (data->info->channels > 2 || data->info->channels < 1) {
        EXCEPTION("Failed to read OGG file: '{}', bad number of channels", data->info->channels);
    }
}

OggFileReader::~OggFileReader() {
    if (data->file.datasource) {
        ov_clear(&data->file);
    }
}

OggFileReader::OggFileReader(OggFileReader&& other) noexcept = default;

OggFileReader& OggFileReader::operator=(OggFileReader&& other) noexcept = default;

AudioFormat Engine::OggFileReader::getFormat() const {
    if (data->info->channels == 1) {
        return AudioFormat::Mono16;
    } else if (data->info->channels == 2) {
        return AudioFormat::Stereo16;
    } else {
        EXCEPTION("Unsupported audio channels: {}", data->info->channels);
    }
}

int Engine::OggFileReader::getFrequency() const {
    return static_cast<int>(data->info->rate);
}

std::vector<uint8_t> Engine::OggFileReader::readData() {
    std::vector<uint8_t> pcm;

    pcm.resize(ov_pcm_total(&data->file, -1) * data->info->channels * 2);
    auto* dst = pcm.data();
    size_t total = 0;

    int section;

    while (true) {
        const auto res = ov_read(&data->file, reinterpret_cast<char*>(dst), 4096, 0, 2, 1, &section);
        if (res == 0) {
            break;
        }

        if (res == OV_EBADLINK) {
            EXCEPTION("Corrupt bitstream audio section");
        }

        dst += res;
        total += res;
    }

    if (total != pcm.size()) {
        EXCEPTION("Failed to read audio data, truncated bitstream");
    }

    return pcm;
}
