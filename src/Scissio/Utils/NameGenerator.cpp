#include "NameGenerator.hpp"
#include <iostream>
#include <sstream>
#include <string_view>

using namespace Scissio;

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

std::string NameGenerator::operator()(std::mt19937_64& rng) {
    const auto length = distLength(rng);

    std::string ss(SEQUENCE_LENGTH + length, ' ');
    std::uniform_int_distribution<size_t> dist;

    const auto start = pick(sequencesKeys, dist(rng));
    for (size_t i = 0; i < start.size(); i++) {
        ss[i] = start[i];
    }

    const auto last = [&](const size_t i) -> std::string { return std::string(&ss[i], SEQUENCE_LENGTH); };

    for (size_t i = 0; i < length; i++) {
        const auto it = sequences.find(last(i));
        if (it != sequences.end()) {
            ss[i + SEQUENCE_LENGTH] = pick(it->second, dist(rng));
        } else {
            break;
        }
    }

    ss[0] = std::toupper(ss[0]);
    return ss;
}

static const std::vector<std::string> MARKOV_CHAIN_NAMES = {
    "Adara",     "Adena",     "Adrianne",  "Alarice",  "Alvita",  "Amara",   "Ambika",    "Antonia",   "Araceli",
    "Balandria", "Basha",     "Beryl",     "Bryn",     "Callia",  "Caryssa", "Cassandra", "Casondrah", "Chatha",
    "Ciara",     "Cynara",    "Cytheria",  "Dabria",   "Darcei",  "Deandra", "Deirdre",   "Delores",   "Desdomna",
    "Devi",      "Dominique", "Drucilla",  "Duvessa",  "Ebony",   "Fantine", "Fuscienne", "Gabi",      "Gallia",
    "Hanna",     "Hedda",     "Jerica",    "Jetta",    "Joby",    "Kacila",  "Kagami",    "Kala",      "Kallie",
    "Keelia",    "Kerry",     "Kerry-Ann", "Kimberly", "Killian", "Kory",    "Lilith",    "Lucretia",  "Lysha",
    "Mercedes",  "Mia",       "Maura",     "Perdita",  "Quella",  "Riona",   "Safiya",    "Salina",    "Severin",
    "Sidonia",   "Sirena",    "Solita",    "Tempest",  "Thea",    "Treva",   "Trista",    "Vala",      "Winta"};

NameGenerator NameGenerator::Default(MARKOV_CHAIN_NAMES);
