#include <TemporaryEscape/Client/Application.hpp>
#include <TemporaryEscape/Utils/Exceptions.hpp>
#include <TemporaryEscape/Utils/Log.hpp>
#include <cxxopts.hpp>

#ifdef _WIN32
#include <Shlobj.h>
#include <Windows.h>
#include <direct.h>
#else
#include <climits>
#include <pwd.h>
#include <unistd.h>
#endif

using namespace Engine;

std::filesystem::path appDataDir() {
#ifdef _WIN32
    char appDataPath[MAX_PATH];
    SHGetSpecialFolderPathA(nullptr, appDataPath, CSIDL_APPDATA, false);
    return std::filesystem::path(std::string(appDataPath)).append("Temporary Escape");
#else
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    return std::filesystem::path(std::string(homedir)).append(".temporary-escape");
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

int main(int argc, char** argv) {
    Log::configure(true);
    Log::i("main", "Temporary Escape main");

    cxxopts::Options parser("Temporary Escape", "Space sim multiplayer game");

    const auto defaultRoot = execDir().parent_path().parent_path();
    const auto defaultUserData = appDataDir();

    parser.add_options()("r,root", "Path to the root game folder",
                         cxxopts::value<std::string>()->default_value(defaultRoot.string()));

    parser.add_options()("u,userdata", "Path to the user data folder",
                         cxxopts::value<std::string>()->default_value(defaultUserData.string()));

    parser.add_options()("save-clean", "Delete the save folder and create a new one");

    parser.add_options()("save-name", "Name of the save folder", cxxopts::value<std::string>());

    try {
        auto args = parser.parse(argc, argv);

        const auto rootPath = std::filesystem::absolute(Path(args["root"].as<std::string>()));
        Config config{};
        config.assetsPath = rootPath / "assets";
        config.userdataPath = std::filesystem::absolute(Path(args["userdata"].as<std::string>()));
        config.userdataSavesPath = config.userdataPath / "Saves";
        config.shadersPath = rootPath / "shaders";

        Application::Options options{};
        if (args.count("save-name")) {
            options.saveFolderName = args["save-name"].as<std::string>();
        }
        options.saveFolderClean = args["save-clean"].as<bool>();

        std::filesystem::create_directories(config.userdataPath);
        std::filesystem::create_directories(config.userdataSavesPath);

        Application application(config, options);
        application.run();
    } catch (const std::exception& e) {
        BACKTRACE("Main", e, "fatal error");
        return EXIT_FAILURE;
    }
}