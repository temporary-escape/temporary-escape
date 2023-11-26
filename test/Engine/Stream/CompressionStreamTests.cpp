#include "../../Common.hpp"
#include <Engine/Stream/MsgpackAcceptor.hpp>
#include <Engine/Stream/MsgpackStream.hpp>
#include <Engine/Utils/Md5.hpp>
#include <msgpack.hpp>

using namespace Engine;

class TestCompressionStream : public CompressionStream {
public:
    explicit TestCompressionStream() : CompressionStream{} {
    }

    void writeCompressed(const char* data, const size_t length) override {
        const auto offset = buffer.size();
        buffer.resize(buffer.size() + length);
        std::memcpy(buffer.data() + offset, data, length);
    }

    std::vector<char> buffer;
};

class TestDecompressionAcceptor : public DecompressionAcceptor {
public:
    TestDecompressionAcceptor() : DecompressionAcceptor{} {
    }

    void writeDecompressed(const char* data, size_t length) override {
        const auto offset = buffer.size();
        buffer.resize(buffer.size() + length);
        std::memcpy(buffer.data() + offset, data, length);
    }

    std::vector<char> buffer;
};

TEST_CASE("Compress and decompress small data", "[stream]") {
    TestCompressionStream compress{};
    std::string msg{"Hello World!"};

    compress.write(msg.data(), msg.size());
    compress.flush();

    REQUIRE(!compress.buffer.empty());

    TestDecompressionAcceptor decompress{};

    decompress.accept(compress.buffer.data(), compress.buffer.size());

    REQUIRE(!decompress.buffer.empty());
    REQUIRE(decompress.buffer.size() == msg.size());

    std::string received{decompress.buffer.data(), decompress.buffer.size()};
    REQUIRE(msg == received);
}

TEST_CASE("Compress and decompress large data", "[stream]") {
    std::vector<char> data;
    data.resize(1024 * 1024 * 16);

    std::mt19937_64 rng{5678};
    std::uniform_int_distribution<int> dist{0, 255};

    // Generate random data
    for (char& i : data) {
        i = static_cast<char>(dist(rng));
    }

    // Calculate MD5 of the generated data
    const auto sum = md5sum(data.data(), data.size());
    REQUIRE(!sum.empty());

    // Compress it
    TestCompressionStream compress{};
    compress.write(data.data(), data.size());

    compress.flush();
    REQUIRE(!compress.buffer.empty());

    // Decompress it
    TestDecompressionAcceptor decompress{};
    decompress.accept(compress.buffer.data(), compress.buffer.size());

    // There should be some decompressed data
    REQUIRE(!decompress.buffer.empty());
    REQUIRE(decompress.buffer.size() == data.size());

    // Get MD5 out of the decompressed data
    const auto sumDec = md5sum(decompress.buffer.data(), decompress.buffer.size());

    // Validate that the decompressed data matches compressed
    REQUIRE(sum == sumDec);
}

TEST_CASE("Compress and decompress large data with small chunks", "[stream]") {
    std::vector<char> data;
    data.resize(1024 * 1024 * 16);

    std::mt19937_64 rng{1234};
    std::uniform_int_distribution<int> dist{0, 255};
    std::uniform_int_distribution<size_t> distSize{16, 1024};

    // Generate random data
    for (char& i : data) {
        i = static_cast<char>(dist(rng));
    }

    // Calculate MD5 of the generated data
    const auto sum = md5sum(data.data(), data.size());
    REQUIRE(!sum.empty());

    // Compress it with small chunks at the time
    TestCompressionStream compress{};
    auto src = data.data();
    auto end = data.data() + data.size();
    while (src < end) {
        const auto randSize = std::min<size_t>(distSize(rng), end - src);
        compress.write(src, randSize);
        src += randSize;
    }

    compress.flush();
    REQUIRE(!compress.buffer.empty());

    // Decompress it with small chunks at the time
    TestDecompressionAcceptor decompress{};
    src = compress.buffer.data();
    end = compress.buffer.data() + compress.buffer.size();
    while (src < end) {
        const auto randSize = std::min<size_t>(distSize(rng), end - src);
        decompress.accept(src, randSize);
        src += randSize;
    }

    // There should be some decompressed data
    REQUIRE(!decompress.buffer.empty());
    REQUIRE(decompress.buffer.size() == data.size());

    // Get MD5 out of the decompressed data
    const auto sumDec = md5sum(decompress.buffer.data(), decompress.buffer.size());

    // Validate that the decompressed data matches compressed
    REQUIRE(sum == sumDec);
}

class TestMsgpackStream : public MsgpackStream {
public:
    explicit TestMsgpackStream() : MsgpackStream{} {
    }

    void writeCompressed(const char* data, const size_t length) override {
        const auto offset = buffer.size();
        buffer.resize(buffer.size() + length);
        std::memcpy(buffer.data() + offset, data, length);
    }

    std::vector<char> buffer;
};

class TestMsgpackAcceptor : public MsgpackAcceptor {
public:
    TestMsgpackAcceptor() : MsgpackAcceptor{} {
    }

    void receiveObject(msgpack::object_handle oh) override {
        objects.push_back(std::move(oh));
    }

    std::vector<msgpack::object_handle> objects;
};

TEST_CASE("Compress and decompress stream of bytes with msgpack stream", "[stream]") {
    TestMsgpackStream compress{};
    TestMsgpackAcceptor decompress{};

    std::string msg = "Hello World!";
    int foo = 42;
    bool bar = true;
    std::vector<uint64_t> baz;

    std::mt19937_64 rng{9725674ULL};
    std::uniform_int_distribution<uint64_t> dist{};
    baz.resize(1024 * 1024 * 8);
    for (size_t i = 0; i < baz.size(); i++) {
        baz[i] = dist(rng);
    }

    msgpack::pack(compress, msg);
    msgpack::pack(compress, foo);
    msgpack::pack(compress, bar);
    msgpack::pack(compress, baz);
    compress.flush();

    REQUIRE(compress.buffer.empty() == false);
    decompress.accept(compress.buffer.data(), compress.buffer.size());

    REQUIRE(decompress.objects.empty() == false);
    REQUIRE(decompress.objects.size() == 4);

    std::string msgDec;
    int fooDec;
    bool barDec;
    std::vector<uint64_t> bazDec;

    decompress.objects[0].get().convert(msgDec);
    decompress.objects[1].get().convert(fooDec);
    decompress.objects[2].get().convert(barDec);
    decompress.objects[3].get().convert(bazDec);

    REQUIRE(msgDec == msg);
    REQUIRE(fooDec == foo);
    REQUIRE(barDec == bar);
    REQUIRE(bazDec == baz);
}

TEST_CASE("Compress and decompress random chunks with msgpack stream", "[stream]") {
    TestMsgpackStream compress{};
    TestMsgpackAcceptor decompress{};

    std::mt19937_64 rng{9725674ULL};
    std::uniform_int_distribution<uint64_t> dist{};
    std::uniform_int_distribution<uint64_t> distLength{128, 1024 * 16};

    const size_t maxTotal = 1024 * 1024 * 64;
    size_t total = 0;

    std::vector<std::vector<char>> originals;

    // Compress and send all buffers
    while (total < maxTotal) {
        originals.emplace_back();
        auto& buffer = originals.back();

        auto length = distLength(rng);
        if (total + length < maxTotal) {
            length = maxTotal - total;
        }

        buffer.resize(length);
        for (size_t i = 0; i < buffer.size(); i++) {
            buffer[i] = dist(rng);
        }

        total += buffer.size();

        msgpack::pack(compress, buffer);
        compress.flush();
    }

    REQUIRE(compress.buffer.empty() == false);
    decompress.accept(compress.buffer.data(), compress.buffer.size());

    REQUIRE(decompress.objects.empty() == false);
    REQUIRE(decompress.objects.size() == originals.size());

    for (size_t i = 0; i < originals.size(); i++) {
        std::vector<char> data;
        decompress.objects[i].get().convert(data);

        REQUIRE(data == originals[i]);
    }
}
