#include "../Common.hpp"
#include <Future.hpp>
#include <Server/Database.hpp>

#define TAG "[Database]"

struct Player {
    std::string name;

    MSGPACK_DEFINE(name);
};

namespace Scissio {
SCHEMA_DEFINE(Player);
}

TEST_CASE("Put Get Delete", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

    auto found = db.get<Player>("1234");
    REQUIRE(found.has_value() == false);

    Player player;
    player.name = "Some Name";
    db.put("1234", player);

    found = db.get<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == player.name);

    db.remove<Player>("1234");

    found = db.get<Player>("1234");
    REQUIRE(found.has_value() == false);
}

TEST_CASE("GetMulti", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

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

TEST_CASE("Seek", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

    for (auto i = 0; i < 10; i++) {
        Player player;
        if (i >= 5) {
            player.name = fmt::format("Some Other Name {}", i);
        } else {
            player.name = fmt::format("Some Name {}", i);
        }
        db.put(player.name, player);
    }

    auto players = db.seek<Player>("Bad prefix");
    REQUIRE(players.empty() == true);

    players = db.seek<Player>("Some");
    REQUIRE(players.size() == 10);

    players = db.seek<Player>("Some Other");
    REQUIRE(players.size() == 5);

    players = db.seek<Player>("Some Unknown");
    REQUIRE(players.empty() == true);
}

TEST_CASE("Delete by prefix", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

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

    auto players = db.seek<Player>("Some");
    REQUIRE(players.size() == 10);

    db.removeByPrefix<Player>("Some Other");

    players = db.seek<Player>("Some");
    REQUIRE(players.size() == 5);

    db.removeByPrefix<Player>("Some");

    players = db.seek<Player>("Some");
    REQUIRE(players.empty());
}

TEST_CASE("Transcation", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

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

TEST_CASE("Transcation retry", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

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

TEST_CASE("Update", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

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
    REQUIRE(res.get() == true);
    REQUIRE(counter == 2);

    const auto found = db.get<Player>("1234");
    REQUIRE(found.has_value() == true);
    REQUIRE(found.value().name == "Some Name 2");
}

struct IndexedPlayer {
    uint64_t uid;
    std::string name;
    bool admin;

    MSGPACK_DEFINE(uid, name, admin);
};

namespace Scissio {
SCHEMA_DEFINE_INDEXED(IndexedPlayer, name, admin);
}

TEST_CASE("Indexes", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

    IndexedPlayer player;
    player.uid = 123;
    player.name = "Hello World";
    player.admin = true;
    db.put("1234", player);

    player = IndexedPlayer{};
    player.uid = 124;
    player.name = "Hello World 2";
    player.admin = true;
    db.put("1235", player);

    auto check = db.get<IndexedPlayer>("1234");
    REQUIRE(check.has_value() == true);

    const auto indexKey =
        SchemaHelper<IndexedPlayer>::getIndexKeyName<decltype(IndexedPlayer::name), &IndexedPlayer::name>();
    auto indexes = db.seek<std::string>(indexKey);

    REQUIRE(indexes.size() == 2);

    auto found = db.getByIndex<&IndexedPlayer::name>(std::string("Hello World"));

    REQUIRE(found.size() == 1);
    REQUIRE(found.back().uid == 123);

    found = db.getByIndex<&IndexedPlayer::admin>(true);

    REQUIRE(found.size() == 2);

    found = db.getByIndex<&IndexedPlayer::name>(std::string("World"));

    REQUIRE(found.empty() == true);
}

struct FooDataV1 {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

namespace Scissio {
template <> const char* SchemaDefinition<FooDataV1>::getName() {
    return "FooData";
}
} // namespace Scissio

struct FooDataV2 {
    std::string msg;
    bool field = true;

    MSGPACK_DEFINE(msg, field);
};

namespace Scissio {
template <> const char* SchemaDefinition<FooDataV2>::getName() {
    return "FooData";
}
} // namespace Scissio

TEST_CASE("Backwards compatibility with missing field", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

    FooDataV1 foo;
    foo.msg = "Hello World";

    db.put("foo", foo);

    auto foo2 = db.get<FooDataV2>("foo");
    REQUIRE(foo2.has_value() == true);
    REQUIRE(foo2.value().msg == "Hello World");
    REQUIRE(foo2.value().field == true);
}

TEST_CASE("Backwards compatibility with extra field", TAG) {
    auto tmpDir = std::make_shared<TmpDir>();
    Database db(tmpDir->value());

    FooDataV2 foo2;
    foo2.msg = "Hello World";

    db.put("foo", foo2);

    auto foo = db.get<FooDataV1>("foo");
    REQUIRE(foo.has_value() == true);
    REQUIRE(foo.value().msg == "Hello World");
}
