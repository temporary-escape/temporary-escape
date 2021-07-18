#pragma once
#include "../Library.hpp"
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Scissio {
class SCISSIO_API NameGenerator {
public:
    static const std::vector<std::string> DefaultWords;

    explicit NameGenerator(const std::vector<std::string>& words = DefaultWords);
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
