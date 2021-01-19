#include "ModManager.hpp"

#include "../Assets/Block.hpp"
#include "../Assets/FontFace.hpp"
#include "../Assets/IconAtlas.hpp"
#include "../Assets/Model.hpp"
#include "../Utils/Log.hpp"

#include <set>

using namespace Scissio;

ModManager::ModManager(AssetManager& assetManager) : assetManager(assetManager) {
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

void ModManager::load(const Path& dir) {
    try {
        Log::v("Loading mod from dir: '{}'", dir.string());
        Manifest manifest{};
        Xml::Document(dir / Path("manifest.xml")).getRoot().convert(manifest);

        Log::v("Loading assets for: '{}'", manifest.name);

        iterateDir(dir / Path("fonts"), {".ttf", ".otf"},
                   [&](const Path& path) { assetManager.load<FontFace>(manifest, path); });

        iterateDir(dir / Path("icons"), {".png"},
                   [&](const Path& path) { assetManager.load<IconAtlas>(manifest, path); });

        iterateDir(dir / Path("models"), {".gltf"},
                   [&](const Path& path) { assetManager.load<Model>(manifest, path); });

        iterateDir(dir / Path("blocks"), {".xml"}, [&](const Path& path) { assetManager.load<Block>(manifest, path); });
    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod from dir: '{}'", dir.string());
    }
}
