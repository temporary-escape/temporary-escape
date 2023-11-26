#include "NameGenerator.hpp"
#include "../Server/Lua.hpp"
#include <iostream>
#include <sol/sol.hpp>
#include <sstream>
#include <string_view>

using namespace Engine;

static bool isLatinChar(const char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool isUpperCase(const char c) {
    return c >= 'A' && c <= 'Z';
}

static bool isLatin(const std::string& word) {
    for (const auto& c : word) {
        if (!isLatinChar(c)) {
            return false;
        }
    }
    return true;
}

static const size_t SEQUENCE_LENGTH = 2;

NameGenerator::NameGenerator(const std::vector<std::string>& words) : distLength(2, 9) {
    build(words);
}

void NameGenerator::build(const std::vector<std::string>& words) {
    std::unordered_map<std::string, std::unordered_set<char>> temp;

    for (auto word : words) {
        if (word.empty() || word.size() < SEQUENCE_LENGTH || !isLatin(word)) {
            continue;
        }

        if (isUpperCase(word[0])) {
            word[0] = std::tolower(word[0]);
        }

        for (size_t i = 0; i < word.size() - SEQUENCE_LENGTH; i++) {
            const auto sequence = word.substr(i, SEQUENCE_LENGTH);

            auto it = temp.find(sequence);
            if (it == temp.end()) {
                it = temp.insert(std::make_pair(sequence, std::unordered_set<char>{})).first;
            }

            it->second.insert(word[i + SEQUENCE_LENGTH]);
        }
    }

    for (const auto& pair : temp) {
        sequencesKeys.push_back(pair.first);
        const auto it = sequences.insert(std::make_pair(pair.first, std::vector<char>{})).first;
        for (const auto& c : pair.second) {
            it->second.push_back(c);
        }
    }
}

template <typename T> static T pick(const std::vector<T>& vec, const size_t rand) {
    return vec.at(rand % vec.size());
}

std::string NameGenerator::operator()(std::mt19937_64& rng) const {
    const auto length = distLength(rng);

    std::string ss;
    ss.reserve(SEQUENCE_LENGTH + length);

    std::uniform_int_distribution<size_t> dist;

    const auto start = pick(sequencesKeys, dist(rng));
    size_t end = 0;

    for (size_t i = 0; i < start.size(); i++) {
        ss.push_back(start[i]);
    }
    end = start.size();

    const auto last = [&](const size_t i) -> std::string { return std::string(&ss[i], SEQUENCE_LENGTH); };

    for (size_t i = 0; i < length; i++) {
        const auto it = sequences.find(last(i));
        if (it != sequences.end()) {
            ss.push_back(pick(it->second, dist(rng)));
        } else {
            break;
        }
    }

    ss[0] = std::toupper(ss[0]);
    return {ss.data(), ss.size()};
}

static std::shared_ptr<NameGenerator> createNameGenerator(sol::as_table_t<std::vector<std::string>> arg) {
    return std::make_shared<NameGenerator>(arg.value());
}

void NameGenerator::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<NameGenerator>("NameGenerator", sol::factories(&createNameGenerator));
    cls["get"] = &NameGenerator::operator();
}
