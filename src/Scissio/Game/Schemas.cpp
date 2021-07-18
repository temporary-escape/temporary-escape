#include "Schemas.hpp"

using namespace Scissio;

void Block::convert(const Xml::Node& n, Block& v) {
    n.child("title").convert(v.title);
    n.child("description").convert(v.description);
    n.child("model").convert(v.model);
    n.child("tier").convert(v.tier);
    n.child("category").convert(v.category);
}

void Asteroid::convert(const Xml::Node& n, Asteroid& v) {
    n.child("title").convert(v.title);
    n.child("description").convert(v.description);

    const auto dist = n.child("distribution");
    dist.child("rarity").convert(v.rarity);
    dist.child("min").convert(v.distMin);
    dist.child("max").convert(v.distMax);

    auto model = n.child("models").child("model");
    while (true) {
        v.models.emplace_back();
        model.convert(v.models.back());

        if (!model.hasNext("model")) {
            break;
        }
        model = model.next("model");
    }
}

void Scissio::createSchemas(Database& db) {
    db.create<Block>();
    db.create<Asteroid>();
    db.create<Faction>();
    db.create<Player>();
    db.create<PlayerBlock>();
    db.create<Galaxy>();
    db.create<Region>();
    db.create<System>();
    db.create<SystemLink>();
    db.create<Sector>();
    db.create<PlayerLocation>();
}
