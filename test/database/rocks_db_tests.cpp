#include "../common.hpp"
#include <engine/database/rocks_db.hpp>
#include <engine/future.hpp>

#define TAG "[RocksDB]"

using namespace Engine;

SCHEMA(SchemaNonVersioned) {
    std::string bar;
    int baz = 0;

    SCHEMA_DEFINE(bar, baz);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("NonVersioned");
};

SCHEMA(SchemaFoo) {
    std::string bar;
    int baz = 0;

    SCHEMA_DEFINE(bar, baz);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("Foo");
};

SCHEMA(SchemaFooV2) {
    std::string msg;
    std::vector<int> bazs;

    SchemaFooV2& operator=(SchemaFoo&& other) {
        msg = std::move(other.bar);
        bazs = {other.baz};
        return *this;
    }

    SCHEMA_DEFINE(msg, bazs);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("Foo");
};

SCHEMA(SchemaFooV3) {
    std::vector<std::string> msgs;
    std::vector<int> bazs;

    SchemaFooV3& operator=(SchemaFoo&& other) {
        msgs = {std::move(other.bar)};
        bazs = {other.baz};
        return *this;
    }

    SchemaFooV3& operator=(SchemaFooV2&& other) {
        msgs = {std::move(other.msg)};
        bazs = std::move(other.bazs);
        return *this;
    }

    SCHEMA_DEFINE(msgs, bazs);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("Foo");
};

SCHEMA_VERSIONS(SchemaFoo, SchemaFooV2, SchemaFooV3)

TEST_CASE("Simple schema put get and delete", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    SchemaFoo foo{};
    foo.bar = "Hello World";
    foo.baz = 42;
    db.put<SchemaFoo>("123456789", foo);

    auto found = db.get<SchemaFoo>("123456789");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().baz == foo.baz);
    REQUIRE(found.value().baz == foo.baz);

    db.remove<SchemaFoo>("123456789");

    found = db.get<SchemaFoo>("123456789");
    REQUIRE(found.has_value() == false);
}

TEST_CASE("Schema versioning", TAG) {
    REQUIRE(Database::getSchemaVersion<SchemaNonVersioned>() == 1ULL);

    REQUIRE(Database::getSchemaVersion<SchemaFoo>() == 1ULL);
    REQUIRE(Database::getSchemaVersion<SchemaFooV2>() == 2ULL);
    REQUIRE(Database::getSchemaVersion<SchemaFooV3>() == 3ULL);
}

TEST_CASE("Schema version conversion", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    SECTION("From V1 to V2") {
        SchemaFoo foo1{};
        foo1.bar = "Hello World";
        foo1.baz = 42;
        db.put<SchemaFoo>("123456789", foo1);

        auto found = db.get<SchemaFooV2>("123456789");
        REQUIRE(found.has_value() == true);
        REQUIRE(found.value().msg == "Hello World");
        REQUIRE(found.value().bazs.size() == 1);
        REQUIRE(found.value().bazs.at(0) == 42);
    }

    SECTION("From V2 to V3") {
        SchemaFooV2 foo2{};
        foo2.msg = "Hello World";
        foo2.bazs = {42};
        db.put<SchemaFooV2>("123456789", foo2);

        auto found = db.get<SchemaFooV3>("123456789");
        REQUIRE(found.has_value() == true);
        REQUIRE(found.value().msgs.size() == 1);
        REQUIRE(found.value().msgs.at(0) == "Hello World");
        REQUIRE(found.value().bazs.size() == 1);
        REQUIRE(found.value().bazs.at(0) == 42);
    }

    SECTION("From V1 to V3") {
        SchemaFoo foo1{};
        foo1.bar = "Hello World";
        foo1.baz = 42;
        db.put<SchemaFoo>("123456789", foo1);

        auto found = db.get<SchemaFooV3>("123456789");
        REQUIRE(found.has_value() == true);
        REQUIRE(found.value().msgs.size() == 1);
        REQUIRE(found.value().msgs.at(0) == "Hello World");
        REQUIRE(found.value().bazs.size() == 1);
        REQUIRE(found.value().bazs.at(0) == 42);
    }

    SECTION("From V3 to V1") {
        SchemaFooV3 foo3{};
        foo3.msgs = {"Hello World"};
        foo3.bazs = {42};
        db.put<SchemaFooV3>("123456789", foo3);

        REQUIRE_THROWS(db.get<SchemaFoo>("123456789"));
    }
}

SCHEMA(Player) {
    std::string name;

    SCHEMA_DEFINE(name);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("Player");
};

TEST_CASE("Use multiGet to get multiple keys at once", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    for (auto i = 0; i < 10; i++) {
        Player player;
        if (i >= 5) {
            player.name = fmt::format("Some Other Name {}", i);
        } else {
            player.name = fmt::format("Some Name {}", i);
        }
        db.put(player.name, player);
    }

    auto players = db.multiGet<Player>({"Some Name 1", "Some Other Name 7"});
    REQUIRE(players.size() == 2);
    REQUIRE(players.at(0).name == "Some Name 1");
    REQUIRE(players.at(1).name == "Some Other Name 7");

    players = db.multiGet<Player>({"Some Other Name 7", "Some Unknown Name", "Some Name 1"});
    REQUIRE(players.size() == 2);
    REQUIRE(players.at(0).name == "Some Other Name 7");
    REQUIRE(players.at(1).name == "Some Name 1");
}

TEST_CASE("Seek many values", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    for (auto i = 0; i < 10; i++) {
        Player player;
        if (i >= 5) {
            player.name = fmt::format("Some Other Name {}", i);
        } else {
            player.name = fmt::format("Some Name {}", i);
        }
        db.put(player.name, player);
    }

    auto it = db.seek<Player>("Bad prefix");
    REQUIRE(it == false);

    size_t count = 0;
    it = db.seek<Player>("Some");
    while (it) {
        count++;
        REQUIRE(it.value.name.find("Some") == 0);
        it.next();
    }
    REQUIRE(count == 10);

    count = 0;
    it = db.seek<Player>("Some Other");
    while (it) {
        count++;
        REQUIRE(it.value.name.find("Some") == 0);
        it.next();
    }
    REQUIRE(count == 5);

    it = db.seek<Player>("Some Unknown");
    REQUIRE(it == false);
}

SCHEMA(SchemaComplexKey) {
    std::string id;

    SCHEMA_NAME("ComplexKey");
    SCHEMA_INDEXES_NONE();
    SCHEMA_DEFINE(id);
};

TEST_CASE("Seek with lower bound", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    const auto parentKey = "Parent";

    for (auto i = 0; i < 10; i++) {
        SchemaComplexKey item;
        item.id = fmt::format("Foo {}", static_cast<char>('A' + i));
        db.put(fmt::format("{}/{}", parentKey, item.id), item);
    }

    const auto collect = [&](const std::string& start) {
        std::vector<SchemaComplexKey> items;
        auto it = db.seek<SchemaComplexKey>(parentKey, start);
        while (it) {
            items.push_back(it.value);
            it.next();
        }
        return items;
    };

    auto items = collect("");
    REQUIRE(items.size() == 10);
    REQUIRE(items.at(0).id == "Foo A");
    REQUIRE(items.at(1).id == "Foo B");

    items = collect("Foo");
    REQUIRE(items.size() == 10);
    REQUIRE(items.at(0).id == "Foo A");
    REQUIRE(items.at(1).id == "Foo B");

    items = collect(fmt::format("{}/Foo C", parentKey));
    REQUIRE(items.size() == 8);
    REQUIRE(items.at(0).id == "Foo C");
    REQUIRE(items.at(1).id == "Foo D");
    REQUIRE(items.at(2).id == "Foo E");
    REQUIRE(items.at(3).id == "Foo F");
    REQUIRE(items.at(4).id == "Foo G");
    REQUIRE(items.at(5).id == "Foo H");
    REQUIRE(items.at(6).id == "Foo I");
    REQUIRE(items.at(7).id == "Foo J");

    items = collect(fmt::format("{}/Foo I", parentKey));
    REQUIRE(items.size() == 2);
    REQUIRE(items.at(0).id == "Foo I");
    REQUIRE(items.at(1).id == "Foo J");

    items = collect(fmt::format("{}/Foo K", parentKey));
    REQUIRE(items.empty());
}

TEST_CASE("Remove all by prefix", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    for (auto i = 0; i < 10; i++) {
        Player player;
        if (i >= 5) {
            player.name = fmt::format("Some Other Name {}", i);
        } else {
            player.name = fmt::format("Some Name {}", i);
        }
        db.put(player.name, player);
    }

    db.removeByPrefix<Player>("Bad prefix");

    const auto collect = [&](const std::string& key) {
        std::vector<Player> items;
        auto it = db.seek<Player>(key);
        while (it) {
            items.push_back(it.value);
            it.next();
        }
        return items;
    };

    auto players = collect("Some");
    REQUIRE(players.size() == 10);

    db.removeByPrefix<Player>("Some Other");

    players = collect("Some");
    REQUIRE(players.size() == 5);

    db.removeByPrefix<Player>("Some");

    players = collect("Some");
    REQUIRE(players.empty());
}

TEST_CASE("Perform a transcation", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    Player player;
    player.name = "Some Name";
    db.put("1234", player);

    Promise<void> signal;
    auto futureSignal = signal.future();

    Promise<void> fetched;
    auto futureFetched = fetched.future();

    auto counter = 0;

    auto res = std::async([&]() {
        const auto r = db.transaction(
            [&](Transaction& txn) {
                counter++;

                txn.getForUpdate<Player>("1234");

                fetched.resolve();
                futureSignal.wait();

                Player player2;
                player2.name = "Some Name 2";
                txn.put("1234", player2);
                return true;
            },
            false);
        return r;
    });

    futureFetched.wait();

    Player player3;
    player3.name = "Some Name 3";
    db.put("1234", player3);

    signal.resolve();

    REQUIRE(res.get() == false);
    REQUIRE(counter == 1);

    const auto found = db.get<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == "Some Name 3");
}

TEST_CASE("Perform a ranscation with retry", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    Player player;
    player.name = "Some Name";
    db.put("1234", player);

    Promise<void> signal;
    auto futureSignal = signal.future();

    Promise<void> fetched;
    auto futureFetched = fetched.future();

    auto resolved = false;

    auto counter = 0;

    auto res = std::async([&]() {
        const auto r = db.transaction(
            [&](Transaction& txn) {
                counter++;

                txn.getForUpdate<Player>("1234");

                if (!resolved) {
                    resolved = true;
                    fetched.resolve();
                    futureSignal.wait();
                }

                Player player2;
                player2.name = "Some Name 2";
                txn.put("1234", player2);
                return true;
            },
            true);
        return r;
    });

    futureFetched.wait();

    Player player3;
    player3.name = "Some Name 3";
    db.put("1234", player3);

    signal.resolve();

    res.wait();
    REQUIRE(res.get() == true);
    REQUIRE(counter == 2);

    const auto found = db.get<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == "Some Name 2");
}

TEST_CASE("Update a single key", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    Player player;
    player.name = "Some Name";
    db.put("1234", player);

    Promise<void> signal;
    auto futureSignal = signal.future();

    Promise<void> fetched;
    auto futureFetched = fetched.future();

    auto resolved = false;

    auto counter = 0;

    auto res = std::async([&]() {
        return db.update<Player>("1234", [&](std::optional<Player>& value) {
            counter++;

            if (!resolved) {
                resolved = true;
                fetched.resolve();
                futureSignal.wait();
            }

            value.value().name = "Some Name 2";
            return true;
        });
    });

    futureFetched.wait();

    Player player3;
    player3.name = "Some Name 3";
    db.put("1234", player3);

    signal.resolve();

    res.wait();
    auto [updated, value] = res.get();
    REQUIRE(updated == true);
    REQUIRE(counter == 2);

    const auto found = db.get<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == "Some Name 2");
}

SCHEMA(SchemaIndexedPlayer) {
    uint64_t uid{0};
    std::string name;
    bool admin{false};

    SCHEMA_DEFINE(uid, name, admin);
    SCHEMA_NAME("IndexedPlayer");
    SCHEMA_INDEXES(name, admin);
};

TEST_CASE("Schema indexes", TAG) {
    REQUIRE(std::string(Database::getSchemaIndexName<SchemaIndexedPlayer, &SchemaIndexedPlayer::name>()) == "name");
    REQUIRE(std::string(Database::getSchemaIndexName<SchemaIndexedPlayer, &SchemaIndexedPlayer::admin>()) == "admin");
    REQUIRE_THROWS(Database::getSchemaIndexName<SchemaIndexedPlayer, &SchemaIndexedPlayer::uid>());

    auto tmpDir = std::make_shared<TmpDir>();
    RocksDB db(tmpDir->value());

    SchemaIndexedPlayer player;
    player.uid = 123;
    player.name = "Hello World";
    player.admin = true;
    db.put("1234", player);

    player = SchemaIndexedPlayer{};
    player.uid = 124;
    player.name = "Hello World 2";
    player.admin = true;
    db.put("1235", player);

    auto check = db.get<SchemaIndexedPlayer>("1234");
    REQUIRE(check.has_value() == true);

    auto found = db.getByIndex<&SchemaIndexedPlayer::name>(std::string("Hello World"));

    REQUIRE(found.size() == 1);
    REQUIRE(found.back().uid == 123);

    found = db.getByIndex<&SchemaIndexedPlayer::admin>(true);

    REQUIRE(found.size() == 2);

    found = db.getByIndex<&SchemaIndexedPlayer::name>(std::string("World"));

    REQUIRE(found.empty() == true);
}
