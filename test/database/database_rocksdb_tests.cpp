#include "../common.hpp"
#include <engine/database/database_rocksdb.hpp>
#include <engine/future.hpp>
#include <engine/utils/random.hpp>

#define TAG "[DatabaseRocksDB]"

using namespace Engine;

struct SchemaFoo {
    std::string bar;
    int baz = 0;

    MSGPACK_DEFINE(bar, baz);
};

SCHEMA_DEFINE(SchemaFoo);

TEST_CASE("Database simple schema put get and delete", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

    SchemaFoo foo{};
    foo.bar = "Hello World";
    foo.baz = 42;
    db.put<SchemaFoo>("123456789", foo);

    auto found = db.find<SchemaFoo>("123456789");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().baz == foo.baz);
    REQUIRE(found.value().baz == foo.baz);

    db.remove<SchemaFoo>("123456789");

    found = db.find<SchemaFoo>("123456789");
    REQUIRE(found.has_value() == false);
}

struct Player {
    std::string name;

    MSGPACK_DEFINE(name);
};

SCHEMA_DEFINE(Player);

TEST_CASE("Database use multiGet to get multiple keys at once", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

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

TEST_CASE("Database seek many values", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

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
    REQUIRE(it.next() == false);

    size_t count = 0;
    it = db.seek<Player>("Some");
    while (it.next()) {
        count++;
        REQUIRE(it.value().name.find("Some") == 0);
    }
    REQUIRE(count == 10);

    count = 0;
    it = db.seek<Player>("Some Other");
    while (it.next()) {
        count++;
        REQUIRE(it.value().name.find("Some") == 0);
    }
    REQUIRE(count == 5);

    it = db.seek<Player>("Some Unknown");
    REQUIRE(it.next() == false);
}

struct SchemaComplexKey {
    std::string id;

    MSGPACK_DEFINE(id);
};

SCHEMA_DEFINE(SchemaComplexKey);

TEST_CASE("Database seek with lower bound", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

    const auto parentKey = "Parent";

    for (auto i = 0; i < 10; i++) {
        SchemaComplexKey item;
        item.id = fmt::format("Foo {}", static_cast<char>('A' + i));
        db.put(fmt::format("{}/{}", parentKey, item.id), item);
    }

    const auto collect = [&](const std::string& start) {
        std::vector<SchemaComplexKey> items;
        auto it = db.seek<SchemaComplexKey>(parentKey, start);
        while (it.next()) {
            items.push_back(it.value());
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

TEST_CASE("Database remove all by prefix", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

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
        while (it.next()) {
            items.push_back(it.value());
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

TEST_CASE("Database perform a transcation", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

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
        return db.transaction([&](Database& txn) {
            counter++;

            txn.get<Player>("1234");

            if (!resolved) {
                resolved = true;
                fetched.resolve();
                futureSignal.wait();
            }

            Player player2;
            player2.name = "Some Name 2";
            txn.put("1234", player2);
            return true;
        });
    });

    futureFetched.wait();

    Player player3;
    player3.name = "Some Name 3";
    db.put("1234", player3);

    signal.resolve();

    res.wait();
    REQUIRE(res.get() == true);
    REQUIRE(counter == 2);

    const auto found = db.find<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == "Some Name 2");
}

TEST_CASE("Database update a single key", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

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
        db.update<Player>("1234", [&](std::optional<Player> value) {
            counter++;

            if (!resolved) {
                resolved = true;
                fetched.resolve();
                futureSignal.wait();
            }

            value.value().name = "Some Name 2";
            return value.value();
        });
    });

    futureFetched.wait();

    Player player3;
    player3.name = "Some Name 3";
    db.put("1234", player3);

    signal.resolve();

    res.wait();
    res.get();
    REQUIRE(counter == 2);

    const auto found = db.find<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == "Some Name 2");
}

struct SchemaIndexedPlayer {
    uint64_t uid{0};
    std::string name;
    bool admin{false};

    MSGPACK_DEFINE(uid, name, admin);
};

SCHEMA_DEFINE(SchemaIndexedPlayer);
SCHEMA_INDEXES(SchemaIndexedPlayer, name, admin);

TEST_CASE("Database schema indexes", TAG) {
    REQUIRE(std::string{Details::getSchemaIndexName<SchemaIndexedPlayer, &SchemaIndexedPlayer::name>()} == "name");
    REQUIRE(std::string{Details::getSchemaIndexName<SchemaIndexedPlayer, &SchemaIndexedPlayer::admin>()} == "admin");
    REQUIRE(Details::hasSchemaIndexName<SchemaIndexedPlayer>("name") == true);
    REQUIRE(Details::hasSchemaIndexName<SchemaIndexedPlayer>("admin") == true);
    REQUIRE(Details::hasSchemaIndexName<SchemaIndexedPlayer>("uid") == false);
    REQUIRE_THROWS(Details::getSchemaIndexName<SchemaIndexedPlayer, &SchemaIndexedPlayer::uid>());

    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

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

    auto check = db.find<SchemaIndexedPlayer>("1234");
    REQUIRE(check.has_value() == true);

    auto found = db.getByIndex<&SchemaIndexedPlayer::name>(std::string("Hello World"));

    REQUIRE(found.size() == 1);
    REQUIRE(found.back().uid == 123);

    found = db.getByIndex<&SchemaIndexedPlayer::admin>(true);

    REQUIRE(found.size() == 2);

    found = db.getByIndex<&SchemaIndexedPlayer::name>(std::string("World"));

    REQUIRE(found.empty() == true);
}

/*struct BenchmarkData {
    std::string msg;
    MSGPACK_DEFINE_ARRAY(msg);
};

TEST_CASE("Benchmark RocksDB", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    DatabaseRocksDB::Options options{};
    DatabaseRocksDB db{tmpDir->value(), options};

    std::vector<std::string> ids;
    ids.resize(1000000);

    for (size_t i = 0; i < ids.size(); i++) {
        ids[i] = uuid();

        BenchmarkData data{};
        data.msg = ids[i];

        msgpack::sbuffer buffer;
        msgpack::pack(buffer, data);

        db.putRaw(ids[i], buffer.data(), buffer.size());
    }

    const auto t0 = std::chrono::steady_clock::now();
    for (size_t i = 0; i < ids.size(); i++) {
        const auto oh = db.getRaw(ids.at(i));
        (void)oh->get().as<BenchmarkData>();
    }
    const auto t1 = std::chrono::steady_clock::now();
    std::cout << "random read: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << "ms"
              << std::endl;
}*/
