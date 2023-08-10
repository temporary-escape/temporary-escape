#include "../../common.hpp"
#include <engine/file/lz4_file_reader.hpp>
#include <engine/file/lz4_file_writer.hpp>

using namespace Engine;

TEST_CASE("Compress and decompress data as LZ4 file", "[lz4_file]") {
    TmpDir dir{};

    std::string msg = "Hello World!\n";

    SECTION("Write by small chunks") {
        Lz4FileWriter file{dir.value() / "file.bin"};
        for (auto i = 0; i < 1000; i++) {
            file.write(msg.data(), msg.size());
        }

        REQUIRE_NOTHROW(file.close());
    }

    SECTION("Write by single large chunk") {
        std::string copy;

        Lz4FileWriter file{dir.value() / "file.bin"};
        for (auto i = 0; i < 1000; i++) {
            copy += msg;
        }

        file.write(copy.data(), copy.size());
        REQUIRE_NOTHROW(file.close());
    }

    { // Read by a small chunk
        Lz4FileReader file{dir.value() / "file.bin"};

        for (auto i = 0; i < 1000; i++) {
            std::string buff;
            buff.resize(msg.size(), 'x');

            REQUIRE(!file.eof());
            REQUIRE(file.read(buff.data(), buff.size()) == buff.size());
            REQUIRE(buff == msg);
        }

        REQUIRE(file.eof());

        char c;
        REQUIRE(file.read(&c, 1) == 0);

        REQUIRE(file.eof());
    }

    { // Read by a large single chunk
        Lz4FileReader file{dir.value() / "file.bin"};

        std::string data;
        data.resize(msg.size() * 1000, 'x');

        REQUIRE(!file.eof());
        REQUIRE(file.read(data.data(), data.size()) == data.size());
        REQUIRE(file.eof());

        for (auto i = 0; i < 1000; i++) {
            const char* src = data.data() + msg.size() * i;
            const auto view = std::string_view{src, msg.size()};
            REQUIRE(view == msg);
        }
    }
}
