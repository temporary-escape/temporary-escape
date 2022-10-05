#include "../../Common/Catch.hpp"
#include <Engine/Scene/Grid.hpp>

#define TAG "[Grid]"

using namespace Engine;

TEST_CASE("Grid pool allocator", TAG) {
    Grid::Pool<uint64_t, 0xFFFF> pool;

    REQUIRE(pool.size() == 0);
    REQUIRE(pool.nextEmpty() == pool.maxSize);

    auto* item = &pool.insert();
    *item = 1;
    REQUIRE(std::distance(pool.data(), item) == 0);
    REQUIRE(pool.size() == 1);
    REQUIRE(pool.nextEmpty() == 1);

    item = &pool.insert();
    *item = 2;
    REQUIRE(std::distance(pool.data(), item) == 1);
    REQUIRE(pool.size() == 2);
    REQUIRE(pool.nextEmpty() == 2);

    for (size_t i = 0; i < 1024; i++) {
        item = &pool.insert();
        *item = i + 3;
    }

    REQUIRE(std::distance(pool.data(), item) == 1024 + 1);
    REQUIRE(pool.size() == 1024 + 2);
    REQUIRE(pool.nextEmpty() == 1024 + 2);

    item = pool.data() + 1024 + 1;
    REQUIRE(*item == 1024 + 2);

    pool.erase(3);
    pool.at(3) = 0;
    REQUIRE(pool.nextEmpty() == 3);
    REQUIRE(pool.size() == 1024 + 1);
}

TEST_CASE("Octree insert voxels at [0, 0, 0]", TAG) {
    Grid::Octree tree;

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.isOutside(Vector3i{0, 0, 0}) == false);
    REQUIRE(tree.isOutside(Vector3i{-1, -1, -1}) == true);
    REQUIRE(tree.isOutside(Vector3i{1, 0, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{-2, 0, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 1, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{-2, -2, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 0, 1}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 0, -2}) == true);

    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == false);

    Grid::Voxel voxel{};
    voxel.type = 1;
    voxel.color = 7;
    voxel.rotation = 13;
    tree.insert(Vector3i{0, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);

    auto opt = tree.find(Vector3i{0, 0, 0});
    REQUIRE(opt.has_value() == true);

    auto found = opt.value();
    REQUIRE(found.type == voxel.type);
    REQUIRE(found.color == voxel.color);
    REQUIRE(found.rotation == voxel.rotation);

    REQUIRE(tree.find(Vector3i{0, 0, -1}).has_value() == false);
    REQUIRE(tree.find(Vector3i{-1, 0, -1}).has_value() == false);
    REQUIRE(tree.find(Vector3i{-1, 0, 0}).has_value() == false);
    REQUIRE(tree.find(Vector3i{0, -1, 0}).has_value() == false);
    REQUIRE(tree.find(Vector3i{0, -1, -1}).has_value() == false);
    REQUIRE(tree.find(Vector3i{-1, -1, -1}).has_value() == false);
    REQUIRE(tree.find(Vector3i{-1, -1, 0}).has_value() == false);

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
}

TEST_CASE("Octree insert voxels at range [-1, -1, -1] - [0, 0, 0]", TAG) {
    Grid::Octree tree;

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.isOutside(Vector3i{0, 0, 0}) == false);
    REQUIRE(tree.isOutside(Vector3i{-1, -1, -1}) == true);
    REQUIRE(tree.isOutside(Vector3i{1, 0, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{-2, 0, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 1, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{-2, -2, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 0, 1}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 0, -2}) == true);
    REQUIRE(tree.size() == 1);

    size_t count = 0;
    for (auto z = -1; z <= 0; z++) {
        for (auto y = -1; y <= 0; y++) {
            for (auto x = -1; x <= 0; x++) {
                Grid::Voxel voxel{};
                voxel.type = count++ + 1;
                voxel.color = 7;
                voxel.rotation = 13;
                tree.insert(Vector3i{x, y, z}, voxel);
            }
        }
    }

    REQUIRE(tree.size() == 17);
    REQUIRE(tree.getDepth() == 2);
    REQUIRE(tree.getWidth() == 2);

    count = 0;
    for (auto z = -1; z <= 0; z++) {
        for (auto y = -1; y <= 0; y++) {
            for (auto x = -1; x <= 0; x++) {
                auto opt = tree.find(Vector3i{x, y, z});
                REQUIRE(opt.has_value() == true);

                auto found = opt.value();
                REQUIRE(found.type == count++ + 1);
                REQUIRE(found.color == 7);
                REQUIRE(found.rotation == 13);
            }
        }
    }

    REQUIRE(tree.size() == 17);
}

TEST_CASE("Octree insert voxels at [1, 0, 0]", TAG) {
    Grid::Octree tree;

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.isOutside(Vector3i{0, 0, 0}) == false);
    REQUIRE(tree.isOutside(Vector3i{-1, -1, -1}) == true);
    REQUIRE(tree.isOutside(Vector3i{1, 0, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{-2, 0, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 1, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{-2, -2, 0}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 0, 1}) == true);
    REQUIRE(tree.isOutside(Vector3i{0, 0, -2}) == true);

    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == false);
    REQUIRE(tree.find(Vector3i{1, 0, 0}).has_value() == false);

    Grid::Voxel voxel{};
    voxel.type = 123;
    voxel.color = 7;
    voxel.rotation = 13;

    tree.insert(Vector3i{1, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 2);
    REQUIRE(tree.getWidth() == 2);

    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == false);

    const auto opt = tree.find(Vector3i{1, 0, 0});
    REQUIRE(opt.has_value() == true);
    const auto& found = opt.value();
    REQUIRE(found.type == 123);
    REQUIRE(found.color == 7);
    REQUIRE(found.rotation == 13);

    REQUIRE(tree.size() == 3);
}

TEST_CASE("Octree insert voxels at [0, 0, 0] and then at [1, 0, 0]", TAG) {
    Grid::Octree tree;

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == false);
    REQUIRE(tree.find(Vector3i{1, 0, 0}).has_value() == false);

    Grid::Voxel voxel{};
    voxel.type = 123;
    voxel.color = 7;
    voxel.rotation = 13;
    tree.insert(Vector3i{0, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == true);
    REQUIRE(tree.find(Vector3i{1, 0, 0}).has_value() == false);

    voxel = Grid::Voxel{};
    voxel.type = 121;
    voxel.color = 6;
    voxel.rotation = 12;
    tree.insert(Vector3i{1, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 2);
    REQUIRE(tree.getWidth() == 2);

    auto opt = tree.find(Vector3i{0, 0, 0});
    REQUIRE(opt.has_value() == true);
    auto found = opt.value();
    REQUIRE(found.type == 123);
    REQUIRE(found.color == 7);
    REQUIRE(found.rotation == 13);

    opt = tree.find(Vector3i{1, 0, 0});
    REQUIRE(opt.has_value() == true);
    found = opt.value();
    REQUIRE(found.type == 121);
    REQUIRE(found.color == 6);
    REQUIRE(found.rotation == 12);

    REQUIRE(tree.size() == 4);
}

TEST_CASE("Octree insert voxels at [0, 0, 0] and then at [2, 0, 0]", TAG) {
    Grid::Octree tree;

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == false);
    REQUIRE(tree.find(Vector3i{2, 0, 0}).has_value() == false);

    Grid::Voxel voxel{};
    voxel.type = 123;
    voxel.color = 7;
    voxel.rotation = 13;
    tree.insert(Vector3i{0, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);
    REQUIRE(tree.find(Vector3i{0, 0, 0}).has_value() == true);
    REQUIRE(tree.find(Vector3i{2, 0, 0}).has_value() == false);

    voxel = Grid::Voxel{};
    voxel.type = 121;
    voxel.color = 6;
    voxel.rotation = 12;
    tree.insert(Vector3i{2, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 3);
    REQUIRE(tree.getWidth() == 4);

    auto opt = tree.find(Vector3i{0, 0, 0});
    REQUIRE(opt.has_value() == true);
    auto found = opt.value();
    REQUIRE(found.type == 123);
    REQUIRE(found.color == 7);
    REQUIRE(found.rotation == 13);

    opt = tree.find(Vector3i{2, 0, 0});
    REQUIRE(opt.has_value() == true);
    found = opt.value();
    REQUIRE(found.type == 121);
    REQUIRE(found.color == 6);
    REQUIRE(found.rotation == 12);

    REQUIRE(tree.size() == 6);
}

TEST_CASE("Octree insert voxels at range [0, 0, 0] - [7, 0, 0]", TAG) {
    Grid::Octree tree;

    REQUIRE(tree.getDepth() == 1);
    REQUIRE(tree.getWidth() == 1);

    for (auto x = 0; x <= 7; x++) {
        Grid::Voxel voxel{};
        voxel.type = x + 1;
        voxel.color = 7;
        voxel.rotation = 13;
        tree.insert(Vector3i{x, 0, 0}, voxel);
    }

    tree.dump();

    REQUIRE(tree.getDepth() == 4);
    REQUIRE(tree.getWidth() == 8);

    for (auto x = 0; x <= 7; x++) {
        auto opt = tree.find(Vector3i{x, 0, 0});
        REQUIRE(opt.has_value() == true);
        auto found = opt.value();
        REQUIRE(found.type == x + 1);
        REQUIRE(found.color == 7);
        REQUIRE(found.rotation == 13);
    }

    for (auto z = -10; z < 10; z++) {
        for (auto y = -10; y < 10; y++) {
            for (auto x = -10; x < 10; x++) {
                if (x >= 0 && x <= 7 && y == 0 && z == 0) {
                    continue;
                }

                const auto opt = tree.find(Vector3i{x, y, z});
                REQUIRE(opt.has_value() == false);
            }
        }
    }
}

TEST_CASE("Octree insert and iterate two blocks", TAG) {
    Grid::Octree tree;

    Grid::Voxel voxel{};
    voxel.type = 1;
    voxel.color = 7;
    voxel.rotation = 13;

    tree.insert(Vector3i{0, 0, 0}, voxel);
    tree.insert(Vector3i{-1, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 2);
    REQUIRE(tree.getWidth() == 2);

    tree.dump();

    auto level0 = tree.iterate();
    REQUIRE(!!level0);
    REQUIRE(level0.getLevel() == 0);
    REQUIRE(level0.getOrigin() == Vector3i{0, 0, 0});
    REQUIRE(level0.getPos() == Vector3i{0, 0, 0});
    REQUIRE(level0.isVoxel() == false);

    auto level1 = level0.children();
    REQUIRE(!!level1);
    REQUIRE(level1.getLevel() == 1);
    REQUIRE(level1.getOrigin() == Vector3i{0, 0, 0});
    REQUIRE(level1.getPos() == Vector3i{1, 1, 1});
    REQUIRE(level1.isVoxel() == false);

    auto children1 = level1.children();

    level1.next();
    REQUIRE(!!level1);

    REQUIRE(level1.getLevel() == 1);
    REQUIRE(level1.getOrigin() == Vector3i{0, 0, 0});
    REQUIRE(level1.getPos() == Vector3i{-1, 1, 1});
    REQUIRE(level1.isVoxel() == false);

    auto children2 = level1.children();

    level1.next();
    REQUIRE(!level1);

    REQUIRE(!!children1);
    REQUIRE(children1.getLevel() == 2);
    REQUIRE(children1.getOrigin() == Vector3i{1, 1, 1});
    REQUIRE(children1.getPos() == Vector3i{0, 0, 0});
    REQUIRE(children1.isVoxel() == true);

    REQUIRE(!!children2);
    REQUIRE(children2.getLevel() == 2);
    REQUIRE(children2.getOrigin() == Vector3i{-1, 1, 1});
    REQUIRE(children2.getPos() == Vector3i{-1, 0, 0});
    REQUIRE(children2.isVoxel() == true);
}

TEST_CASE("Octree insert and iterate three blocks", TAG) {
    Grid::Octree tree;

    Grid::Voxel voxel{};
    voxel.type = 1;
    voxel.color = 7;
    voxel.rotation = 13;

    tree.insert(Vector3i{0, 0, 0}, voxel);
    tree.insert(Vector3i{-1, 0, 0}, voxel);
    tree.dump();
    tree.insert(Vector3i{2, 0, 0}, voxel);

    REQUIRE(tree.getDepth() == 3);
    REQUIRE(tree.getWidth() == 4);

    tree.dump();

    auto level0 = tree.iterate();
    REQUIRE(!!level0);
    REQUIRE(level0.getLevel() == 0);
    REQUIRE(level0.getOrigin() == Vector3i{0, 0, 0});
    REQUIRE(level0.getPos() == Vector3i{0, 0, 0});
    REQUIRE(level0.isVoxel() == false);

    auto level1 = level0.children();
    REQUIRE(!!level1);
    REQUIRE(level1.getLevel() == 1);
    REQUIRE(level1.getOrigin() == Vector3i{0, 0, 0});
    REQUIRE(level1.getPos() == Vector3i{2, 2, 2});
    REQUIRE(level1.isVoxel() == false);

    auto level2 = level1.children();
    REQUIRE(!!level2);
    REQUIRE(level2.getLevel() == 2);
    REQUIRE(level2.getOrigin() == Vector3i{2, 2, 2});
    REQUIRE(level2.getPos() == Vector3i{1, 1, 1});
    REQUIRE(level2.isVoxel() == false);

    auto level3 = level2.children();
    REQUIRE(!!level3);
    REQUIRE(level3.getLevel() == 3);
    REQUIRE(level3.getOrigin() == Vector3i{1, 1, 1});
    REQUIRE(level3.getPos() == Vector3i{0, 0, 0});
    REQUIRE(level3.isVoxel() == true);
    level3.next();
    REQUIRE(!level3);

    level2.next();
    REQUIRE(!!level2);
    REQUIRE(level2.getLevel() == 2);
    REQUIRE(level2.getOrigin() == Vector3i{2, 2, 2});
    REQUIRE(level2.getPos() == Vector3i{3, 1, 1});
    REQUIRE(level2.isVoxel() == false);

    level3 = level2.children();
    REQUIRE(!!level3);
    REQUIRE(level3.getLevel() == 3);
    REQUIRE(level3.getOrigin() == Vector3i{3, 1, 1});
    REQUIRE(level3.getPos() == Vector3i{2, 0, 0});
    REQUIRE(level3.isVoxel() == true);
    level3.next();
    REQUIRE(!level3);

    level1.next();
    REQUIRE(!!level1);

    REQUIRE(level1.getLevel() == 1);
    REQUIRE(level1.getOrigin() == Vector3i{0, 0, 0});
    REQUIRE(level1.getPos() == Vector3i{-2, 2, 2});
    REQUIRE(level1.isVoxel() == false);

    level2 = level1.children();
    REQUIRE(!!level2);
    REQUIRE(level2.getLevel() == 2);
    REQUIRE(level2.getOrigin() == Vector3i{-2, 2, 2});
    REQUIRE(level2.getPos() == Vector3i{-1, 1, 1});
    REQUIRE(level2.isVoxel() == false);

    level3 = level2.children();
    REQUIRE(!!level3);
    REQUIRE(level3.getLevel() == 3);
    REQUIRE(level3.getOrigin() == Vector3i{-1, 1, 1});
    REQUIRE(level3.getPos() == Vector3i{-1, 0, 0});
    REQUIRE(level3.isVoxel() == true);
    level3.next();
    REQUIRE(!level3);

    level2.next();
    REQUIRE(!level2);

    level1.next();
    REQUIRE(!level1);

    level0.next();
    REQUIRE(!level0);
}

TEST_CASE("Octree insert expand twice", TAG) {
    Grid::Octree tree;

    Grid::Voxel voxel{};
    voxel.type = 1;
    voxel.color = 7;
    voxel.rotation = 13;

    tree.insert(Vector3i{0, 0, 0}, voxel);
    tree.insert(Vector3i{-1, 0, 0}, voxel);
    tree.insert(Vector3i{1, 0, 0}, voxel);
    tree.dump();
    tree.insert(Vector3i{-1, 0, 1}, voxel);
    tree.dump();

    auto opt = tree.find(Vector3i{0, 0, 1});
    REQUIRE(opt.has_value() == false);

    opt = tree.find(Vector3i{1, 0, 1});
    REQUIRE(opt.has_value() == false);

    opt = tree.find(Vector3i{-1, 0, 1});
    REQUIRE(opt.has_value() == true);

    // Root
    auto it = tree.iterate();
    REQUIRE(!!it);

    // First level
    it = it.children();
    REQUIRE(!!it);
    it.next();
    REQUIRE(!!it);

    // Second level
    it = it.children();
    REQUIRE(!!it);
    // [-1, 0, 0]
    REQUIRE(it.isVoxel() == true);
    REQUIRE(it.value().voxel.index == 7);
    REQUIRE(it.getPos() == Vector3i{-1, 0, 0});

    it.next();
    REQUIRE(!!it);

    // [-1, 0, 1]
    REQUIRE(it.isVoxel() == true);
    REQUIRE(it.value().voxel.index == 4);
    REQUIRE(it.getPos() == Vector3i{-1, 0, 1});
}
