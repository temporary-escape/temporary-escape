#include "Game/Application.hpp"
#include "Utils/Exceptions.hpp"

#include <argagg/argagg.hpp>
#include <iostream>

#ifdef _WIN32
#include <Shlobj.h>
#include <Windows.h>
#include <direct.h>
#else
#include <climits>
#include <pwd.h>
#include <unistd.h>
#endif

using namespace Scissio;

argagg::parser argparser{{
    {"help", {"-h", "--help"}, "Shows this help message", 0},
    {"root", {"-r", "--root"}, "Override the game root folder path", 1},
    {"userdata", {"--userdata"}, "Override the game user data folder path", 1},
    {"generate-api-txt", {"--generate-api-txt"}, "Generate mod API documentation in text format", 1},
    {"generate-api-json", {"--generate-api-json"}, "Generate mod API documentation in JSON format", 1},
}};

std::filesystem::path appDataDir() {
#ifdef _WIN32
    char appDataPath[MAX_PATH];
    SHGetSpecialFolderPathA(nullptr, appDataPath, CSIDL_APPDATA, false);
    return std::filesystem::path(std::string(appDataPath)).append("Scissio");
#else
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    return std::filesystem::path(std::string(homedir)).append(".Scissio");
#endif
}

std::filesystem::path execDir() {
#ifdef _WIN32
    std::string exePath;
    exePath.resize(MAX_PATH);
    GetModuleFileNameA(nullptr, &exePath[0], static_cast<DWORD>(exePath.size()));
    exePath.resize(std::strlen(exePath.c_str()));
    return std::filesystem::path(exePath).parent_path();
#else
    std::string result;
    result.resize(PATH_MAX);
    std::string szTmp;
    szTmp.resize(32);
    sprintf(&szTmp[0], "/proc/%d/exe", getpid());
    int bytes = std::min(readlink(&szTmp[0], &result[0], result.size()), ssize_t(result.size() - 1));
    if (bytes >= 0)
        result[bytes] = '\0';
    result.resize(std::strlen(result.c_str()));
    return std::filesystem::path(result).parent_path();
#endif
}

int main(const int argc, char** argv) {
    argagg::parser_results args;
    try {
        args = argparser.parse(argc, argv);

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (args["help"]) {
        std::cerr << argparser;
        return EXIT_SUCCESS;
    }

    try {
        Config config;
        const auto root = std::filesystem::absolute(args["root"] ? Path(args["root"].as<std::string>())
                                                                 : execDir().parent_path().parent_path());

        config.assetsPath = root / "assets";
        config.resourcesPath = root / "resources";
        config.cwdPath = execDir();
        config.userdataPath = args["userdata"] ? Path(args["userdata"].as<std::string>()) : appDataDir();
        config.userdataSavesPath = config.userdataPath / Path("Saves");
        config.shadersPath = config.assetsPath / Path("shaders");

        Application application(config);

        application.run();
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        BACKTRACE(e, "fatal error");
        return EXIT_FAILURE;
    }
}
