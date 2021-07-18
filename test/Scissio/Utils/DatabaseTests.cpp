#include "../Common.hpp"
#include <Utils/Database.hpp>

struct Faction {
    uint64_t id{0};
    std::string name;

    DB_TABLE_NAME("faction");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("name").text().nonNull().unique(),
    });

    DB_BIND(name);
};

struct Player {
    uint64_t id{0};
    std::optional<int64_t> factionId;
    bool admin{false};
    std::string name;
    float reputation{0.0f};

    DB_TABLE_NAME("player");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("factionId").integer().references<Faction>("id").indexed().onDeleteNull(),
        SchemaField("admin").boolean().nonNull(),
        SchemaField("name").text().nonNull().unique(),
        SchemaField("reputation").real().nonNull().defval("0.0"),
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

    // Insert with ID
    Faction faction2{123, "Lorem Ipsum 2"};
    db.insert<Faction>(faction2);
    REQUIRE(faction2.id == 123);

    factions = db.select<Faction>("WHERE id = ?", 123);
    REQUIRE(factions.size() == 1);
    REQUIRE(factions.at(0).id == faction2.id);
    REQUIRE(factions.at(0).name == faction2.name);
}

TEST("Inner join") {
    Database db;

    db.create<Faction>();
    db.create<Player>();

    REQUIRE(Faction::dbSelect() == "faction.name");
    const auto s = Player::dbSelect();
    REQUIRE(s == "player.factionId,player.admin,player.name,player.reputation");

    Faction f0{0, "Lorem Ipsum"};
    Faction f1{0, "Donor"};

    db.insert(f0);
    db.insert(f1);

    Player p0{0, f0.id, false, "Player A", 1.0f};
    Player p1{0, f1.id, false, "Player B", 1.0f};
    Player p2{0, f1.id, false, "Player C", 1.0f};

    db.insert(p0);
    db.insert(p1);
    db.insert(p2);

    const auto join = db.join<Player, Faction>("factionId", "id");
    REQUIRE(join.size() == 3);
}

TEST("Constraint failed") {
    Database db;

    db.create<Faction>();

    Faction faction{0, "Hello"};

    db.insert(faction);
    faction.id = 0;

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
            faction.id = 0;
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
    REQUIRE(player.factionId == std::nullopt);
}

TEST("Update single row") {
    Database db;

    db.create<Faction>();

    Faction faction{0, "Hello"};
    db.insert(faction);
    REQUIRE(faction.id != 0);

    faction.name = "World";
    db.update(faction);

    faction = db.select<Faction>().front();
    REQUIRE(faction.name == "World");
}

TEST("Set multiple rows") {
    Database db;

    db.create<Faction>();
    db.create<Player>();

    Faction faction{0, "Lorem Ipsum"};
    db.insert<Faction>(faction);
    REQUIRE(faction.id != 0);

    Player playerA{0, faction.id, true, "Player A", 1.0f};
    Player playerB{0, faction.id, false, "Player B", 1.9f};
    Player playerC{0, faction.id, false, "Player C", 0.1f};
    db.insert(playerA);
    db.insert(playerB);
    db.insert(playerC);

    db.set<Player>("reputation = ? WHERE admin = ?", 0.0f, false);

    const auto players = db.select<Player>();
    REQUIRE(players.size() == 3);

    const auto pA = std::find_if(players.begin(), players.end(), [](const auto& p) { return p.name == "Player A"; });
    const auto pB = std::find_if(players.begin(), players.end(), [](const auto& p) { return p.name == "Player B"; });
    const auto pC = std::find_if(players.begin(), players.end(), [](const auto& p) { return p.name == "Player C"; });

    REQUIRE(pA != players.end());
    REQUIRE(pA->admin == true);
    REQUIRE(pA->reputation > 0.0f);

    REQUIRE(pB != players.end());
    REQUIRE(pB->admin == false);
    REQUIRE(pB->reputation == Approx(0.0f));

    REQUIRE(pC != players.end());
    REQUIRE(pC->admin == false);
    REQUIRE(pC->reputation == Approx(0.0f));

    REQUIRE(db.count<Faction>() == 1);
    REQUIRE(db.count<Player>() == 3);
}

TEST("Describe") {
    Database db;

    REQUIRE(db.exists<Faction>() == false);
    REQUIRE(db.exists<Player>() == false);

    db.create<Faction>();
    db.create<Player>();

    REQUIRE(db.exists<Faction>() == true);
    REQUIRE(db.exists<Player>() == true);

    const auto schema = db.describe("player");
    REQUIRE(schema == Player::dbSchema());
}

/*TEST("Describe with compound indexes") {
    Database db;

    db.create<Faction>();
    db.create<Player>();

    db.exec(R"(
    CREATE TABLE example (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        playerId INTEGER NOT NULL,
        factionId INTEGER NOT NULL,
        FOREIGN KEY (playerId) REFERENCES player (id),
        FOREIGN KEY (factionId) REFERENCES faction (id)
    );
    CREATE UNIQUE INDEX example_playerId_factionId_idx ON example(playerId, factionId);
    )");

    const auto schema = db.describe("example");
}*/

TEST("Describe non existing table") {
    Database db;

    REQUIRE_THROWS(db.describe("player"));
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
    REQUIRE(player.factionId == std::nullopt);
}

struct SomeSchemaV1 {
    uint64_t id{0};
    int64_t fieldA{0};
    bool fieldB{false};
    std::string fieldC{false};

    DB_TABLE_NAME("SomeSchema");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("fieldA").integer().nonNull().indexed(),
        SchemaField("fieldB").boolean().nonNull(),
        SchemaField("fieldC").text().nonNull(),
    });

    DB_BIND(fieldA, fieldB, fieldC);
};

struct SomeSchemaV2 {
    uint64_t id{0};
    std::optional<int64_t> fieldA{0};
    std::string fieldC{false};
    float fieldD{0.0f};

    DB_TABLE_NAME("SomeSchema");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("fieldA").integer().indexed(),
        SchemaField("fieldC").text().nonNull().unique(),
        SchemaField("fieldD").real().defval("1.0"),
    });

    DB_BIND(fieldA, fieldC, fieldD);
};

TEST("Auto migrate table") {
    Database db;

    db.create<SomeSchemaV1>();

    SomeSchemaV1 v0{0, 123, true, "Hello"};
    SomeSchemaV1 v1{0, 456, false, "World"};
    db.insert(v0);
    db.insert(v1);

    db.create<SomeSchemaV2>();
    const auto vs = db.select<SomeSchemaV2>();
    REQUIRE(vs.size() == 2);

    const SomeSchemaV2* v0b;
    const SomeSchemaV2* v1b;

    if (vs.at(0).id == v0.id) {
        v0b = &vs.at(0);
        v1b = &vs.at(1);
    } else {
        v0b = &vs.at(1);
        v1b = &vs.at(0);
    }

    REQUIRE(v0b->fieldA.has_value() == true);
    REQUIRE(v0b->fieldA.value() == 123);
    REQUIRE(v0b->fieldC == "Hello");
    REQUIRE(v0b->fieldD == Approx(1.0f));

    REQUIRE(v1b->fieldA.has_value() == true);
    REQUIRE(v1b->fieldA.value() == 456);
    REQUIRE(v1b->fieldC == "World");
    REQUIRE(v1b->fieldD == Approx(1.0f));

    // Sanity check unique keys
    SomeSchemaV2 v3{0, 789, "World", 3.0f};
    REQUIRE_THROWS_WITH(db.insert(v3), Catch::Contains("UNIQUE constraint failed: SomeSchema.fieldC"));
}

struct SomeItem {
    uint64_t id{0};
    std::string key;
    std::string name;

    DB_TABLE_NAME("SomeItem");

    DB_SCHEMA({
        SchemaField("id").integer().primary().autoInc(),
        SchemaField("key").text().nonNull().unique(),
        SchemaField("name").text().nonNull(),
    });

    DB_BIND(key, name);
};

/*TEST("Replace existing") {
    Database db;

    db.create<SomeItem>();

    SomeItem a{0, "aaa", "Hello"};
    SomeItem b{0, "bbb", "World"};

    db.replace(a, "key");
    db.replace(b, "key");

    auto items = db.select<SomeItem>();
    REQUIRE(items.size() == 2);

    items = db.select<SomeItem>("WHERE key = ?", std::string("bbb"));
    REQUIRE(items.size() == 1);
    REQUIRE(items.front().id == 2);
    REQUIRE(items.front().name == "World");

    SomeItem c{0, "bbb", "Hello World!"};

    db.replace(c, "key");

    items = db.select<SomeItem>();
    REQUIRE(items.size() == 2);

    items = db.select<SomeItem>("WHERE key = ?", std::string("bbb"));
    REQUIRE(items.size() == 1);
    REQUIRE(items.front().id == 2);
    REQUIRE(items.front().name == "Hello World!");
}*/
