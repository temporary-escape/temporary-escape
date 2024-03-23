#include "SaveInfo.hpp"
#include "../Utils/Log.hpp"
#include <regex>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

struct Version {
    int mayor{0};
    int minor{0};
    int patch{0};
};

static std::optional<Version> stringToVersion(const std::string& text) {
    static const auto pattern = std::regex(R"(^v(\d+)\.(\d+)\.(\d+).*$)");
    std::smatch m;
    if (std::regex_search(text, m, pattern) && m.size() == 4) {
        return Version{
            std::stoi(m[1]),
            std::stoi(m[2]),
            std::stoi(m[3]),
        };
    }

    return std::nullopt;
}

static bool isCompatibleVersion(const std::string& version) {
    static const auto ours = stringToVersion(GAME_VERSION);
    assert(ours);

    const auto their = stringToVersion(version);
    return their && ours->mayor == their->mayor && ours->minor == their->minor && their->patch <= ours->patch;
}

std::vector<Engine::SaveInfo> Engine::loadSaveInfoDir(const Path& dir) {
    std::vector<SaveInfo> results;

    if (Fs::exists(dir) && Fs::is_directory(dir)) {
        for (const auto& it : Fs::directory_iterator(dir)) {
            if (it.is_directory()) {
                const auto path = Fs::absolute(it.path() / "info.xml");

                try {
                    results.emplace_back();
                    Xml::fromFile(path, results.back());

                    results.back().path = it.path();
                    results.back().compatible = isCompatibleVersion(results.back().version);
                } catch (std::exception& e) {
                    BACKTRACE(e, "Failed to load save info: '{}'", path);
                }
            }
        }
    }

    return results;
}
