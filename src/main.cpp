#include <CLI/CLI.hpp>
#include <engine/client/application.hpp>
#include <engine/utils/exceptions.hpp>
#include <engine/utils/log.hpp>

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

int main(int argc, char** argv) {
    const auto defaultRoot = getExecutablePath();
    const auto defaultUserData = getAppDataPath();
    const auto defaultPythonHome = getExecutablePath() / "python";

    Config config{};
    CLI::App parser{"Temporary Escape"};

    Path rootPath;
    parser.add_option("--root", rootPath, "Root directory")
        ->check(CLI::ExistingDirectory)
        ->default_val(defaultRoot.string());
    parser.add_option("--python-home", config.pythonHome, "Python lib directory")
        ->check(CLI::ExistingDirectory)
        ->default_val(defaultPythonHome.string());
    parser.add_option("--width", config.windowWidth, "Window width");
    parser.add_option("--height", config.windowHeight, "Window height");

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
        logger.info("Temporary Escape main");
        logger.info("Log file location: '{}'", logPath.string());

        rootPath = std::filesystem::absolute(rootPath);
        config.assetsPath = rootPath / "assets";
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
            Application window{config};
            window.run();
        }
        logger.info("Exit success");
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        BACKTRACE(e, "fatal error");
        return EXIT_FAILURE;
    }
}
