#pragma once
#include "../Library.hpp"
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Engine {
class ENGINE_API NameGenerator {
public:
    explicit NameGenerator(const std::vector<std::string>& words);
    virtual ~NameGenerator() = default;

    std::string operator()(std::mt19937_64& rng) const;

    static void bind(Lua& lua);

private:
    void build(const std::vector<std::string>& words);

    std::unordered_map<std::string, std::vector<char>> sequences;
    std::vector<std::string> sequencesKeys;
    mutable std::uniform_int_distribution<size_t> distLength;
};
} // namespace Engine
