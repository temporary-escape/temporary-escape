#include "../common.hpp"
#include <engine/file/msgpack_file_reader.hpp>
#include <engine/file/msgpack_file_writer.hpp>

using namespace Engine;

struct ExampleStruct {
    std::string msg;
    std::vector<int> data;
    bool flag{false};

    MSGPACK_DEFINE(msg, data, flag);
};

TEST_CASE("Pack and unpack msgpack struct as LZ4 file", "[msgpack_file]") {
    TmpDir dir{};

    ExampleStruct s{};
    s.msg = "Hello World!";
    s.data = {1, 9, 42, 10};
    s.flag = true;

    { // Write
        MsgpackFileWriter file{dir.value() / "file.bin"};
        file.pack(s);
    }

    { // Read
        MsgpackFileReader file{dir.value() / "file.bin"};
        ExampleStruct test{};
        file.unpack(test);

        REQUIRE(test.msg == s.msg);
        REQUIRE(test.data == s.data);
        REQUIRE(test.flag == s.flag);
    }
}

TEST_CASE("Pack and unpack multiple msgpack structs as LZ4 file", "[msgpack_file]") {
    TmpDir dir{};

    { // Write
        MsgpackFileWriter file{dir.value() / "file.bin"};
        for (auto i = 0; i < 1000; i++) {
            ExampleStruct data{};
            data.msg = "Hello World!";
            data.data = {1, 9, 42, i};
            data.flag = true;

            file.pack(data);
        }
    }

    { // Read
        MsgpackFileReader file{dir.value() / "file.bin"};

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
