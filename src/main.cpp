#include <CLI/CLI.hpp>
#include <engine/client/application.hpp>
#include <engine/utils/exceptions.hpp>
#include <engine/utils/log.hpp>

using namespace Engine;

int main(int argc, char** argv) {
    const auto defaultRoot = std::filesystem::absolute(getExecutablePath() / ".." / "..");
    const auto defaultUserData = getAppDataPath();

    CLI::App parser{"Temporary Escape"};

    Path rootPath;
    parser.add_option("--root", rootPath, "Root directory")->check(CLI::ExistingDirectory)->default_val(defaultRoot);

    CLI11_PARSE(parser, argc, argv);

    try {
        /*auto args = parser.parse(argc, argv);

        if (args["version"].as<bool>()) {
            std::cout << "v0.0.1" << std::endl;
            return EXIT_SUCCESS;
        }

        const auto userdataPath = std::filesystem::absolute(Path(args["userdata"].as<std::string>()));
        const auto logPath = userdataPath / "debug.log";

        Log::configure(true, logPath);
        Log::i("main", "Temporary Escape main");
        Log::i("main", "Log file location: '{}'", logPath.string());

        const auto rootPath = std::filesystem::absolute(Path(args["root"].as<std::string>()));
        Config config{};
        config.assetsPath = rootPath / "assets";
        config.wrenPaths = {config.assetsPath.string()};
        config.userdataPath = std::filesystem::absolute(Path(args["userdata"].as<std::string>()));
        config.userdataSavesPath = config.userdataPath / "Saves";
        config.shadersPath = rootPath / "shaders";

        if (args.count("save-name")) {
            config.saveFolderName = args["save-name"].as<std::string>();
        }
        config.saveFolderClean = args["save-clean"].as<bool>();
        config.voxelTest = args["voxel-test"].as<bool>();

        std::filesystem::create_directories(config.userdataPath);
        std::filesystem::create_directories(config.userdataSavesPath);*/

        const auto userdataPath = std::filesystem::absolute(Path(defaultUserData));
        const auto logPath = userdataPath / "debug.log";

        // Log::configure(true, logPath);
        Log::i("main", "Temporary Escape main");
        Log::i("main", "Log file location: '{}'", logPath.string());

        rootPath = std::filesystem::absolute(rootPath);
        Config config{};
        config.assetsPath = rootPath / "assets";
        config.wrenPaths = {config.assetsPath.string()};
        config.userdataPath = std::filesystem::absolute(defaultUserData);
        config.userdataSavesPath = config.userdataPath / "Saves";
        config.shaderCachePath = config.userdataPath / "Shaders";
        config.shadersPath = rootPath / "shaders";
        config.fontsPath = rootPath / "fonts";
        config.shapesPath = rootPath / "shapes";

        std::filesystem::create_directories(config.userdataPath);
        std::filesystem::create_directories(config.userdataSavesPath);
        std::filesystem::create_directories(config.shaderCachePath);

        {
            Application window(config);
            window.run();
        }
        Log::i("main", "Exit success");
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        BACKTRACE("Main", e, "fatal error");
        return EXIT_FAILURE;
    }
}
