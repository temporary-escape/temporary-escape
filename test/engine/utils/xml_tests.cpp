#include "../../common.hpp"
#include <engine/utils/xml.hpp>

using namespace Engine;

TEST_CASE("Write xml document with empty root", "[xml]") {
    Xml::Document doc{"root", "1.0"};
    const auto str = doc.toString();

    static const std::string expected = R"(<?xml version="1.0" encoding="utf-8"?>
<root/>
)";

    REQUIRE(str == expected);
}

struct DocFoo {
    struct Nested {
        int value;
        bool flag;

        void convert(const Xml::Node& xml) {
            xml.convert("value", value);
            xml.convert("flag", flag);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("value", value);
            xml.pack("flag", flag);
        }
    };

    std::vector<std::string> messages;
    Vector3i vec;
    Nested nested;

    void convert(const Xml::Node& xml) {
        xml.convert("message", messages);
        xml.convert("vec", vec);
        xml.convert("nested", nested);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("message", messages);
        xml.pack("vec", vec);
        xml.pack("nested", nested);
    }
};

XML_DEFINE(DocFoo, "foo");

TEST_CASE("Write and read struct document", "[xml]") {
    DocFoo foo{};
    foo.messages = {"Hello World!", "Another message!"};
    foo.vec = Vector3i{3, 42, 0};
    foo.nested.value = 99;
    foo.nested.flag = true;

    const auto str = Xml::dump(foo);

    static const std::string expected = R"(<?xml version="1.0" encoding="utf-8"?>
<foo>
  <message>Hello World!</message>
  <message>Another message!</message>
  <vec>
    <x>3</x>
    <y>42</y>
    <z>0</z>
  </vec>
  <nested>
    <value>99</value>
    <flag>true</flag>
  </nested>
</foo>
)";

    REQUIRE(str == expected);

    DocFoo parsed{};
    Xml::load(parsed, str);
    REQUIRE(parsed.messages.size() == 2);
    REQUIRE(parsed.messages[0] == foo.messages[0]);
    REQUIRE(parsed.messages[1] == foo.messages[1]);
    REQUIRE(parsed.nested.value == foo.nested.value);
    REQUIRE(parsed.nested.flag == foo.nested.flag);
    REQUIRE(parsed.vec == foo.vec);
}

TEST_CASE("Xml throw if unable to parse property", "[xml]") {
    SECTION("Missing property") {
        static const std::string raw = R"(<?xml version="1.0" encoding="utf-8"?>
<foo>
  <vec>
    <x>3</x>
  </vec>
  <nested>
    <value>99</value>
    <flag>true</flag>
  </nested>
</foo>
)";

        DocFoo foo{};
        REQUIRE_THROWS_WITH(Xml::load(foo, raw), "Missing property: foo/vec/y");
    }

    SECTION("Invalid integer property") {
        static const std::string raw = R"(<?xml version="1.0" encoding="utf-8"?>
<foo>
  <vec>
    <x>3</x>
    <y>aaaaa</y>
    <z>3</z>
  </vec>
  <nested>
    <value>99</value>
    <flag>true</flag>
  </nested>
</foo>
)";

        DocFoo foo{};
        REQUIRE_THROWS_WITH(Xml::load(foo, raw), "Unable to parse property: foo/vec/y error: invalid int32_t value");
    }
}

struct DocBaz {
    struct Nested {
        std::string msg;
        int value;

        void convert(const Xml::Node& xml) {
            xml.convert("msg", msg);
            xml.convert("value", value);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("msg", msg);
            xml.pack("value", value);
        }
    };

    std::vector<Nested> items;

    void convert(const Xml::Node& xml) {
        xml.convert("item", items);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("item", items);
    }
};

XML_DEFINE(DocBaz, "baz");

TEST_CASE("Complex nested xml document", "[xml]") {
    DocBaz baz{};
    baz.items = {
        DocBaz::Nested{"Hello World!", 10},
        DocBaz::Nested{"Some message!", 42},
    };

    const auto str = Xml::dump(baz);

    static const std::string expected = R"(<?xml version="1.0" encoding="utf-8"?>
<baz>
  <item>
    <msg>Hello World!</msg>
    <value>10</value>
  </item>
  <item>
    <msg>Some message!</msg>
    <value>42</value>
  </item>
</baz>
)";

    REQUIRE(str == expected);
}

struct DocWithOptional {
    struct Nested {
        std::string msg;

        void convert(const Xml::Node& xml) {
            xml.convert("msg", msg);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("msg", msg);
        }
    };

    std::optional<Nested> value;

    void convert(const Xml::Node& xml) {
        xml.convert("value", value);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("value", value);
    }
};

XML_DEFINE(DocWithOptional, "doc");

TEST_CASE("Xml with optional container", "[xml]") {
    SECTION("Dump with optional value present") {
        DocWithOptional doc{};
        doc.value = DocWithOptional::Nested{"Hello World!"};

        const auto str = Xml::dump(doc);

        static const std::string expected = R"(<?xml version="1.0" encoding="utf-8"?>
<doc>
  <value>
    <msg>Hello World!</msg>
  </value>
</doc>
)";

        REQUIRE(str == expected);
    }

    SECTION("Load with optional value present") {
        static const std::string raw = R"(<?xml version="1.0" encoding="utf-8"?>
<doc>
  <value>
    <msg>Hello World!</msg>
  </value>
</doc>
)";

        DocWithOptional doc{};
        Xml::load(doc, raw);

        REQUIRE(doc.value.has_value());
        REQUIRE(doc.value.value().msg == "Hello World!");
    }

    SECTION("Dump with optional value missing") {
        DocWithOptional doc{};
        doc.value = std::nullopt;

        const auto str = Xml::dump(doc);

        static const std::string expected = R"(<?xml version="1.0" encoding="utf-8"?>
<doc/>
)";

        REQUIRE(str == expected);
    }

    SECTION("Load with optional value missing") {
        static const std::string raw = R"(<?xml version="1.0" encoding="utf-8"?>
<doc/>
)";

        DocWithOptional doc{};
        Xml::load(doc, raw);

        REQUIRE(!doc.value.has_value());
    }

    SECTION("Load with optional value missing empty element") {
        static const std::string raw = R"(<?xml version="1.0" encoding="utf-8"?>
<doc>
  <value/>
</doc>
)";

        DocWithOptional doc{};
        Xml::load(doc, raw);

        REQUIRE(!doc.value.has_value());
    }
}

struct DocWithMap {
    struct Nested {
        std::string msg;

        void convert(const Xml::Node& xml) {
            xml.convert("msg", msg);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("msg", msg);
        }
    };

    std::unordered_map<std::string, Nested> items;

    void convert(const Xml::Node& xml) {
        xml.convert("items", items);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("items", items);
    }
};

XML_DEFINE(DocWithMap, "doc");

TEST_CASE("Xml with unordered_map", "[xml]") {
    DocWithMap doc{};
    doc.items = {
        {"first", {"Hello World 1!"}},
        {"second", {"Hello World 2!"}},
        {"third", {"Hello World 3!"}},
    };

    const auto str = Xml::dump(doc);

    DocWithMap parsed{};
    Xml::load(parsed, str);

    REQUIRE(parsed.items.size() == doc.items.size());
    REQUIRE(parsed.items.find("first") != parsed.items.end());
    REQUIRE(parsed.items.find("second") != parsed.items.end());
    REQUIRE(parsed.items.find("third") != parsed.items.end());
    REQUIRE(parsed.items["first"].msg == doc.items["first"].msg);
    REQUIRE(parsed.items["second"].msg == doc.items["second"].msg);
    REQUIRE(parsed.items["third"].msg == doc.items["third"].msg);
}

struct DocWithEnum {
    enum class SomeEnum {
        None,
        Hello,
        World,
    };

    SomeEnum value{SomeEnum::None};

    void convert(const Xml::Node& xml) {
        xml.convert("value", value);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("value", value);
    }
};

XML_DEFINE(DocWithEnum, "doc");
XML_DEFINE_ENUM(DocWithEnum::SomeEnum, None, Hello, World);

TEST_CASE("Xml with enum", "[xml]") {
    DocWithEnum doc{};
    doc.value = DocWithEnum::SomeEnum::Hello;

    const auto str = Xml::dump(doc);

    static const std::string expected = R"(<?xml version="1.0" encoding="utf-8"?>
<doc>
  <value>Hello</value>
</doc>
)";

    REQUIRE(str == expected);

    DocWithEnum parsed{};
    Xml::load(parsed, str);

    REQUIRE(parsed.value == doc.value);
}

struct DocWithNonRequired {
    struct Nested {
        std::string msg;

        void convert(const Xml::Node& xml) {
            xml.convert("msg", msg, false);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("msg", msg);
        }
    };

    Nested nested;

    void convert(const Xml::Node& xml) {
        xml.convert("nested", nested);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("nested", nested);
    }
};

XML_DEFINE(DocWithNonRequired, "doc");

TEST_CASE("Xml with optional property", "[xml]") {
    static const std::string raw = R"(<?xml version="1.0" encoding="utf-8"?>
<doc>
  <nested/>
</doc>
)";

    DocWithNonRequired parsed{};
    Xml::load(parsed, raw);

    REQUIRE(parsed.nested.msg.empty());
}
