#include "../Common.hpp"
#include <Utils/Database.hpp>

struct Faction {
    uint64_t id{0};
    std::string name;

    DB_TABLE_NAME("faction");

    DB_SCHEMA({
        TableField("id").integer().primary().autoInc(),
        TableField("name").text().nonNull().unique(),
    });

    DB_BIND(name);
};

struct Player {
    uint64_t id{0};
    uint64_t factionId{0};
    bool admin{false};
    std::string name;
    float reputation{0.0f};

    DB_TABLE_NAME("player");

    DB_SCHEMA({
        TableField("id").integer().primary().autoInc(),
        TableField("factionId").integer().references<Faction>("id").indexed().onDeleteNull(),
        TableField("admin").boolean().nonNull(),
        TableField("name").text().nonNull().unique(),
        TableField("reputation").real().nonNull().default("0.0"),
    });

    DB_BIND(factionId, admin, name, reputation);
};

TEST("Validate fields") {
    const auto& fields = Faction::dbSchema().fields;
    REQUIRE(fields.size() == 2);
    REQUIRE(fields.at(0).name == "id");
}

TEST("Basic CRUD operations") {
    Database db;

    db.create<Faction>();
    db.create<Player>();

    Faction faction{0, "Lorem Ipsum"};

    db.insert<Faction>(faction);
    REQUIRE(faction.id != 0);

    auto factions = db.select<Faction>();
    REQUIRE(factions.size() == 1);
    REQUIRE(factions.at(0).id == faction.id);
    REQUIRE(factions.at(0).name == faction.name);

    factions = db.select<Faction>("WHERE id = ?", 123);
    REQUIRE(factions.size() == 0);

    factions = db.select<Faction>("WHERE id = ?", faction.id);
    REQUIRE(factions.size() == 1);
    REQUIRE(factions.at(0).id == faction.id);
    REQUIRE(factions.at(0).name == faction.name);

    auto opt = db.get<Faction>(123);
    REQUIRE(opt.has_value() == false);

    opt = db.get<Faction>(faction.id);
    REQUIRE(opt.has_value() == true);
    REQUIRE(opt.value().id == faction.id);
    REQUIRE(opt.value().name == faction.name);
}

TEST("Constraint failed") {
    Database db;

    db.create<Faction>();

    Faction faction{0, "Hello"};

    db.insert(faction);
    REQUIRE_THROWS_WITH(db.insert(faction), Catch::Contains("UNIQUE constraint failed: faction.name"));

    const auto factions = db.select<Faction>();
    REQUIRE(factions.size() == 1);
}

TEST("Transactions") {
    Database db;

    db.create<Faction>();

    Faction faction{0, "Hello"};

    const auto f = [&]() {
        db.transaction([&]() {
            db.insert(faction);
            db.insert(faction);
        });
    };

    REQUIRE_THROWS_WITH(f(), Catch::Contains("UNIQUE constraint failed: faction.name"));

    const auto factions = db.select<Faction>();
    REQUIRE(factions.size() == 0);
}

TEST("Foreign key on delete null") {
    Database db;

    REQUIRE(db.pragma<bool>("foreign_keys") == true);

    db.create<Faction>();
    db.create<Player>();

    Faction faction{0, "Hello"};
    db.insert(faction);
    REQUIRE(faction.id != 0);

    Player player{0, faction.id, false, "Player", 1.0f};
    db.insert(player);
    REQUIRE(player.id != 0);
    REQUIRE(player.factionId != 0);

    db.remove<Faction>(faction.id);

    player = db.get<Player>(player.id).value();
    REQUIRE(player.factionId == 0);
}

TEST("Move table") {
    Database db;

    REQUIRE(db.pragma<bool>("foreign_keys") == true);

    db.create<Faction>();
    db.create<Player>();

    Faction faction{0, "Hello"};
    db.insert(faction);

    Player player{0, faction.id, false, "Player", 1.0f};
    db.insert(player);

    player = db.get<Player>(player.id).value();
    REQUIRE(player.factionId != 0);

    db.exec(R"(
    PRAGMA foreign_keys=off;
    BEGIN TRANSACTION;
    CREATE TEMPORARY TABLE temp(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL);
    INSERT INTO temp SELECT id,name FROM faction;
    DROP TABLE faction;
    CREATE TABLE faction(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT NOT NULL);
    INSERT INTO faction SELECT id,name FROM temp;
    DROP TABLE temp;
    COMMIT;
    PRAGMA foreign_keys=on;
    )");

    player = db.get<Player>(player.id).value();
    REQUIRE(player.factionId != 0);

    db.remove<Faction>(faction.id);

    player = db.get<Player>(player.id).value();
    REQUIRE(player.factionId == 0);
}
