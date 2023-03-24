#include "../common.hpp"
#include <engine/utils/lz4_file.hpp>
#include <engine/utils/msgpack_adaptors.hpp>

#define TEST_TAG "[lz4_file_tests]"

using namespace Engine;

TEST_CASE("Compress and decompress data as LZ4 file", TEST_TAG) {
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

struct ExampleStruct {
    std::string msg;
    std::vector<int> data;
    bool flag{false};

    MSGPACK_DEFINE(msg, data, flag);
};

TEST_CASE("Pack and unpack msgpack struct as LZ4 file", TEST_TAG) {
    TmpDir dir{};

    ExampleStruct s{};
    s.msg = "Hello World!";
    s.data = {1, 9, 42, 10};
    s.flag = true;

    { // Write
        Lz4FileWriter file{dir.value() / "file.bin"};
        file.pack(s);
    }

    { // Read
        Lz4FileReader file{dir.value() / "file.bin"};
        ExampleStruct test{};
        file.unpack(test);

        REQUIRE(test.msg == s.msg);
        REQUIRE(test.data == s.data);
        REQUIRE(test.flag == s.flag);
    }
}

TEST_CASE("Pack and unpack multiple msgpack structs as LZ4 file", TEST_TAG) {
    TmpDir dir{};

    { // Write
        Lz4FileWriter file{dir.value() / "file.bin"};
        for (auto i = 0; i < 1000; i++) {
            ExampleStruct data{};
            data.msg = "Hello World!";
            data.data = {1, 9, 42, i};
            data.flag = true;

            file.pack(data);
        }
    }

    { // Read
        Lz4FileReader file{dir.value() / "file.bin"};

        for (auto i = 0; i < 1000; i++) {
            ExampleStruct expected{};
            expected.msg = "Hello World!";
            expected.data = {1, 9, 42, i};
            expected.flag = true;

            ExampleStruct test{};
            file.unpack(test);

            REQUIRE(test.msg == expected.msg);
            REQUIRE(test.data == expected.data);
            REQUIRE(test.flag == expected.flag);
        }

        REQUIRE(file.eof());
        ExampleStruct end{};
        REQUIRE(file.unpack(end) == false);
        REQUIRE(file.eof());
    }
}
