#include "../Common.hpp"
#include <TemporaryEscape/Scene/Octree.hpp>

#define TAG "[Octree]"

TEST_CASE("Insert at zero depth then expand", TAG) {
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

TEST_CASE("Ray cast #1", TAG) {
    const Vector3 from{-1.80790210f, 1.61972129f, 2.19639850f};
    const Vector3 to{56.6261482f, -37.6193161f, -71.3039246f};

    Octree<int> tree;
    tree.insert({0, 0, 0});
    tree.insert({1, 0, 0});
    tree.insert({2, 0, 0});

    const auto res = tree.rayCast(from, to);
    REQUIRE(res.has_value());
    REQUIRE(glm::distance(Vector3{-0.140433669f, 0.5f, 0.0990004539f}, res.value().pos) < 0.05f);
    REQUIRE(res.value().offset == 1); // {0, 0, 0}
}

TEST_CASE("Ray cast #2", TAG) {
    const Vector3 from{-2.31813049f, 0.834588826f, 0.807334304f};
    const Vector3 to{96.6625977f, -14.8718967f, -22.7634792f};

    Octree<int> tree;
    tree.insert({0, 0, 0});
    tree.insert({1, 0, 0});
    tree.insert({2, 0, 0});

    const auto res = tree.rayCast(from, to);
    REQUIRE(res.has_value());
    REQUIRE(glm::distance(Vector3{-0.209584713f, 0.5f, 0.305214942f}, res.value().pos) < 0.05f);
    REQUIRE(res.value().offset == 1); // {0, 0, 0}
}

TEST_CASE("Ray cast #3", TAG) {
    const Vector3 from{6.01735115f, 0.846191764f, 0.0433584750f};
    const Vector3 to{-94.3871002f, -12.2351160f, 0.299766093f};

    Octree<int> tree;
    tree.insert({0, 0, 0});
    tree.insert({1, 0, 0});
    tree.insert({2, 0, 0});

    const auto res = tree.rayCast(from, to);
    REQUIRE(res.has_value());
    REQUIRE(glm::distance(Vector3{2.5f, 0.387929648f, 0.0523409024f}, res.value().pos) < 0.05f);
    REQUIRE(res.value().offset == 5); // {2, 0, 0}
}
