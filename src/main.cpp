#include <CLI/CLI.hpp>
#include <engine/client/application.hpp>
#include <engine/utils/exceptions.hpp>
#include <engine/utils/log.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static int commandPlay(const Config& config) {
    {
        Application window{config};
        window.run();
    }
    logger.info("Exit success");
    return EXIT_SUCCESS;
}

static int commandCompressAssets(const Config& config) {
    { AssetsManager::compressAssets(config); }
    logger.info("Exit success");
    return EXIT_SUCCESS;
}

#if defined(_WIN32)
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, const int cmdshow) {
    (void)hInst;
    (void)hInstPrev;
    (void)cmdline;
    (void)cmdshow;
#else
int main(int argc, char** argv) {
#endif
    const auto defaultRoot = getExecutablePath();
    const auto defaultUserData = getAppDataPath();

    Config config{};
    CLI::App parser{"Temporary Escape"};

    Path rootPath;
    parser.add_option("--root", rootPath, "Root directory")
        ->check(CLI::ExistingDirectory)
        ->default_val(defaultRoot.string());
    parser.add_option("--width", config.graphics.windowWidth, "Window width");
    parser.add_option("--height", config.graphics.windowHeight, "Window height");

    parser.add_subcommand("play", "Play the game")->fallthrough(true);
    parser.add_subcommand("compress-assets", "Compress all PNG textures into KTX2")->fallthrough(true);

#if defined(_WIN32)
    CLI11_PARSE(parser, __argc, __argv);
#else
    CLI11_PARSE(parser, argc, argv);
#endif

    try {
        const auto userdataPath = std::filesystem::absolute(Path(defaultUserData));
        const auto logPath = userdataPath / "debug.log";

        // Log::configure(true, logPath);
        logger.info("Temporary Escape main");
        logger.info("Log file location: '{}'", logPath.string());

        rootPath = std::filesystem::absolute(rootPath);
        config.assetsPath = rootPath / "assets";
        config.userdataPath = std::filesystem::absolute(defaultUserData);
        config.userdataSavesPath = config.userdataPath / "Saves";
        config.fontsPath = rootPath / "fonts";
        config.shapesPath = rootPath / "shapes";

        std::filesystem::create_directories(config.userdataPath);
        std::filesystem::create_directories(config.userdataSavesPath);

        if (std::getenv("VK_LAYER_PATH") == nullptr && config.graphics.enableValidationLayers) {
            logger.warn("Vulkan validation layers requested but no env value 'VK_LAYER_PATH' provided");
            config.graphics.enableValidationLayers = false;
        }

        if (parser.got_subcommand("compress-assets")) {
            return commandCompressAssets(config);
        } else if (parser.got_subcommand("play")) {
            return commandPlay(config);
        } else {
            return commandPlay(config);
        }

    } catch (const std::exception& e) {
        BACKTRACE(e, "fatal error");
        return EXIT_FAILURE;
    }
}
