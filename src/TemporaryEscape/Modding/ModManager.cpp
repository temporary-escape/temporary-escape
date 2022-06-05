#include "ModManager.hpp"
#include <set>

#define CMP "ModManager"

using namespace Engine;

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

        manifest.fromYaml(dir / "manifest.yml");
        manifest.path = dir;

        Log::i(CMP, "Loading assets for: '{}' @{}", manifest.name, manifest.version);

        iterateDir(dir / Path("fonts"), {".ttf", ".otf"},
                   [&](const Path& path) { assetManager.addFontFace(manifest, path); });

        iterateDir(dir / Path("models"), {".gltf"}, [&](const Path& path) { assetManager.addModel(manifest, path); });

        iterateDir(dir / Path("shapes"), {".gltf"}, [&](const Path& path) { assetManager.addShape(manifest, path); });

        iterateDir(dir / Path("textures"), {".png"},
                   [&](const Path& path) { assetManager.addTexture(manifest, path, TextureType::Generic); });

        iterateDir(dir / Path("particles"), {".yml"},
                   [&](const Path& path) { assetManager.addParticles(manifest, path); });

        iterateDir(dir / Path("images"), {".png"}, [&](const Path& path) { assetManager.addImage(manifest, path); });

        iterateDir(dir / Path("planets"), {".yml"}, [&](const Path& path) { assetManager.addPlanet(manifest, path); });

        iterateDir(dir / Path("blocks"), {".yml"}, [&](const Path& path) { assetManager.addBlock(manifest, path); });

        iterateDir(dir / Path("turrets"), {".yml"}, [&](const Path& path) { assetManager.addTurret(manifest, path); });

        iterateDir(dir / Path("entities"), {".wren"},
                   [&](const Path& path) { assetManager.addEntity(manifest, path); });

        iterateDir(dir / Path("sectors"), {".wren"}, [&](const Path& path) { assetManager.addSector(manifest, path); });

    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod assets from dir: '{}'", dir.string());
    }
}
