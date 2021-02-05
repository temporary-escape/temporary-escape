#pragma once
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Scissio {
class NameGenerator {
public:
    explicit NameGenerator(const std::vector<std::string>& words);
    virtual ~NameGenerator() = default;

    std::string operator()(std::mt19937_64& rng);

    static NameGenerator Default;

private:
    void build(const std::vector<std::string>& words);

    std::unordered_map<std::string, std::vector<char>> sequences;
    std::vector<std::string> sequencesKeys;
    std::uniform_int_distribution<size_t> distLength;
};
} // namespace Scissio
