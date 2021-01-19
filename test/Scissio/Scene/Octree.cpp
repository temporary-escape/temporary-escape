#include "../Common.hpp"
#include <Scene/Octree.hpp>

TEST("Insert at zero depth then expand") {
    Octree<int> tree;
    auto ref = tree.insert({0, 0, 0});
    REQUIRE(ref.node.parent == 0);
    REQUIRE(ref.offset == 1);

    REQUIRE(tree.find({0, 0, 0}) != std::nullopt);
    REQUIRE(tree.find({1, 0, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 0, 1}) == std::nullopt);
    REQUIRE(tree.find({0, 0, 1}) == std::nullopt);

    REQUIRE(tree.find({0, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 1}) == std::nullopt);
    REQUIRE(tree.find({0, 1, 1}) == std::nullopt);

    auto ref2 = tree.insert({1, 1, 1});
    REQUIRE(ref2.node.parent == 0);
    REQUIRE(ref2.offset == 2);

    REQUIRE(tree.find({0, 0, 0}) != std::nullopt);
    REQUIRE(tree.find({1, 0, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 0, 1}) == std::nullopt);
    REQUIRE(tree.find({0, 0, 1}) == std::nullopt);

    REQUIRE(tree.find({0, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 1}) != std::nullopt);
    REQUIRE(tree.find({0, 1, 1}) == std::nullopt);

    auto ref3 = tree.insert({2, 1, 1});
    REQUIRE(ref3.node.parent == 3);
    REQUIRE(ref3.offset == 5);

    REQUIRE(tree.find({0, 0, 0}) != std::nullopt);
    REQUIRE(tree.find({1, 0, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 0, 1}) == std::nullopt);
    REQUIRE(tree.find({0, 0, 1}) == std::nullopt);

    REQUIRE(tree.find({0, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 1}) != std::nullopt);
    REQUIRE(tree.find({0, 1, 1}) == std::nullopt);

    REQUIRE(tree.find({2, 1, 1}) != std::nullopt);

    auto& root = tree.root();
    REQUIRE(root.children[0] != 0);
    REQUIRE(root.children[6] != 0);

    auto ref4 = tree.insert({3, 1, 1});
    REQUIRE(ref4.node.parent == 8);

    REQUIRE(tree.find({0, 0, 0}) != std::nullopt);
    REQUIRE(tree.find({1, 0, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 0, 1}) == std::nullopt);
    REQUIRE(tree.find({0, 0, 1}) == std::nullopt);

    REQUIRE(tree.find({0, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 0}) == std::nullopt);
    REQUIRE(tree.find({1, 1, 1}) != std::nullopt);
    REQUIRE(tree.find({0, 1, 1}) == std::nullopt);

    REQUIRE(tree.find({2, 1, 1}) != std::nullopt);
    REQUIRE(tree.find({3, 1, 1}) != std::nullopt);
}
