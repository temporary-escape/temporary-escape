#include "ModManager.hpp"
#include <set>

#define CMP "ModManager"

using namespace Scissio;

ModManager::ModManager() {
}

template <typename Fn> static void iterateDir(const Path& dir, const std::set<std::string>& exts, const Fn& fn) {
    if (Fs::exists(dir) && Fs::is_directory(dir)) {
        for (const auto& it : Fs::recursive_directory_iterator(dir)) {
            const auto ext = it.path().extension().string();
            if (it.is_regular_file() && exts.find(ext) != exts.end()) {
                const auto path = Fs::absolute(it.path());
                fn(path);
            }
        }
    }
}

void ModManager::load(AssetManager& assetManager, const Path& dir) {
    try {
        Log::i(CMP, "Loading mod from dir: '{}'", dir.string());
        manifests.push_back(std::make_shared<Manifest>());
        Manifest& manifest = *manifests.back();

        Xml::Document(dir / Path("manifest.xml")).getRoot().convert(manifest);

        Log::i(CMP, "Loading assets for: '{}'", manifest.name);

        iterateDir(dir / Path("fonts"), {".ttf", ".otf"},
                   [&](const Path& path) { assetManager.addFontFace(manifest, path); });

        iterateDir(dir / Path("models"), {".gltf"}, [&](const Path& path) { assetManager.addModel(manifest, path); });

        iterateDir(dir / Path("textures"), {".png"},
                   [&](const Path& path) { assetManager.addTexture(manifest, path, TextureType::Generic); });

        iterateDir(dir / Path("images"), {".png"}, [&](const Path& path) { assetManager.addImage(manifest, path); });

        iterateDir(dir / Path("planets"), {".xml"}, [&](const Path& path) { assetManager.addPlanet(manifest, path); });

        iterateDir(dir / Path("asteroids"), {".xml"},
                   [&](const Path& path) { assetManager.addAsteroid(manifest, path); });

    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod assets from dir: '{}'", dir.string());
    }
}
