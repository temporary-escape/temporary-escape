#include "../common.hpp"
#include <engine/utils/yaml.hpp>
#include <fstream>

#define TAG "[Yaml]"

using namespace Engine;

template <typename T> static void saveAndLoad(T& v) {
    auto path = tmpEmptyFile();
    v.toYaml(path);

    std::ifstream t(path);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    std::cout << "Contents of file: " << path << std::endl;
    std::cout << "BEGIN\n" << str << "\nEND" << std::endl;

    v = T{};
    v.fromYaml(path);
}

TEST_CASE("Map basic C++ types to yaml document", TAG) {
    struct Foo {
        int number = 0;
        float real = 0;
        std::string text;
        bool flag = false;

        YAML_DEFINE(number, real, text, flag);
    };

    Foo foo{};
    foo.fromYaml(asFile(R"(
number: 42
real: 1.234
text: 'Hello World!'
flag: true
)"));

    REQUIRE(foo.number == 42);
    REQUIRE(foo.real == Approx(1.234f));
    REQUIRE(foo.text == "Hello World!");
    REQUIRE(foo.flag == true);

    saveAndLoad(foo);

    REQUIRE(foo.number == 42);
    REQUIRE(foo.real == Approx(1.234f));
    REQUIRE(foo.text == "Hello World!");
    REQUIRE(foo.flag == true);
}

TEST_CASE("Optional", TAG) {
    struct Foo {
        std::optional<std::string> text;
        bool flag = false;

        YAML_DEFINE(text, flag);
    };

    Foo foo{};
    foo.fromYaml(asFile(R"(
flag: true
)"));

    REQUIRE(foo.text.has_value() == false);
    REQUIRE(foo.flag == true);

    foo = Foo{};
    foo.fromYaml(asFile(R"(
text: null
flag: true
)"));

    REQUIRE(foo.text.has_value() == false);
    REQUIRE(foo.flag == true);

    foo = Foo{};
    foo.fromYaml(asFile(R"(
text: 'Hello World!'
flag: true
)"));

    REQUIRE(foo.text.has_value() == true);
    REQUIRE(foo.text.value() == "Hello World!");
    REQUIRE(foo.flag == true);

    saveAndLoad(foo);

    REQUIRE(foo.text.has_value() == true);
    REQUIRE(foo.text.value() == "Hello World!");
    REQUIRE(foo.flag == true);

    foo.text = std::nullopt;
    saveAndLoad(foo);

    REQUIRE(foo.text.has_value() == false);
    REQUIRE(foo.flag == true);
}

TEST_CASE("Vectors", TAG) {
    struct Foo {
        std::vector<std::string> items;

        YAML_DEFINE(items);
    };

    Foo foo = Foo{};
    foo.fromYaml(asFile(R"(
items:
    - "first"
    - "second"
    - "third"
)"));

    REQUIRE(foo.items.size() == 3);
    REQUIRE(foo.items.at(0) == "first");
    REQUIRE(foo.items.at(1) == "second");
    REQUIRE(foo.items.at(2) == "third");

    saveAndLoad(foo);

    REQUIRE(foo.items.size() == 3);
    REQUIRE(foo.items.at(0) == "first");
    REQUIRE(foo.items.at(1) == "second");
    REQUIRE(foo.items.at(2) == "third");
}

TEST_CASE("Maps", TAG) {
    struct Foo {
        std::unordered_map<std::string, std::string> items;

        YAML_DEFINE(items);
    };

    Foo foo = Foo{};
    foo.fromYaml(asFile(R"(
items:
    first: "first item"
    second: "second item"
    third: "third item"
)"));

    REQUIRE(foo.items.size() == 3);
    REQUIRE(foo.items.at("first") == "first item");
    REQUIRE(foo.items.at("second") == "second item");
    REQUIRE(foo.items.at("third") == "third item");

    saveAndLoad(foo);

    REQUIRE(foo.items.size() == 3);
    REQUIRE(foo.items.at("first") == "first item");
    REQUIRE(foo.items.at("second") == "second item");
    REQUIRE(foo.items.at("third") == "third item");
}

enum class EnumBaz {
    None,
    First,
    Second,
    Third,
};

YAML_DEFINE_ENUM(EnumBaz, None, First, Second, Third)

TEST_CASE("Enums", TAG) {
    struct Foo {
        EnumBaz baz;

        YAML_DEFINE(baz);
    };

    Foo foo = Foo{};
    foo.fromYaml(asFile(R"(
baz: "Third"
)"));

    REQUIRE(foo.baz == EnumBaz::Third);
}

class VariantTypeA {
public:
    std::string msg;

    YAML_DEFINE(msg);
};

class VariantTypeB {
public:
    bool flag{false};

    YAML_DEFINE(flag);
};

class VariantTypeC {
public:
    float value{0.0f};

    YAML_DEFINE(value);
};

using VariantType = std::variant<VariantTypeA, VariantTypeB, VariantTypeC>;
YAML_DEFINE_VARIANT(VariantType, VariantTypeA, VariantTypeB, VariantTypeC);

class VariantUser {
public:
    std::vector<VariantType> variants;

    YAML_DEFINE(variants);
};

TEST_CASE("Variants", TAG) {
    // convertEach("aaa", "bbb", "ccc");

    VariantUser v{};
    v.fromYaml(asFile(R"(
variants:
    - type: VariantTypeA
      data:
        msg: "Hello World!"
    - type: VariantTypeB
      data:
        flag: true
    - type: VariantTypeC
      data:
        value: 42.123
)"));

    REQUIRE(v.variants.size() == 3);
    REQUIRE(std::holds_alternative<VariantTypeA>(v.variants[0]));
    REQUIRE(std::holds_alternative<VariantTypeB>(v.variants[1]));
    REQUIRE(std::holds_alternative<VariantTypeC>(v.variants[2]));
    REQUIRE(std::get<VariantTypeA>(v.variants[0]).msg == "Hello World!");
    REQUIRE(std::get<VariantTypeB>(v.variants[1]).flag == true);
    REQUIRE(std::get<VariantTypeC>(v.variants[2]).value == Approx(42.123f));

    saveAndLoad(v);

    REQUIRE(v.variants.size() == 3);
    REQUIRE(std::holds_alternative<VariantTypeA>(v.variants[0]));
    REQUIRE(std::holds_alternative<VariantTypeB>(v.variants[1]));
    REQUIRE(std::holds_alternative<VariantTypeC>(v.variants[2]));
    REQUIRE(std::get<VariantTypeA>(v.variants[0]).msg == "Hello World!");
    REQUIRE(std::get<VariantTypeB>(v.variants[1]).flag == true);
    REQUIRE(std::get<VariantTypeC>(v.variants[2]).value == Approx(42.123f));
}
