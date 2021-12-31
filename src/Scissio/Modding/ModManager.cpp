#include "ModManager.hpp"

#include "../Assets/AssetFontFace.hpp"
#include "../Assets/AssetModel.hpp"
#include "../Utils/Log.hpp"

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

        iterateDir(dir / Path("planets"), {".xml"}, [&](const Path& path) { assetManager.addPlanet(manifest, path); });

        /*iterateDir(dir / Path("icons"), {".png"},
                   [&](const Path& path) { assetManager.load<IconAtlas>(manifest, path); });

        iterateDir(dir / Path("models"), {".gltf"},
                   [&](const Path& path) { assetManager.load<Model>(manifest, path); });

        iterateDir(dir / Path("textures"), {".png"},
                   [&](const Path& path) { assetManager.load<BasicTexture>(manifest, path); });

        iterateDir(dir / Path("images"), {".png"}, [&](const Path& path) { assetManager.load<Image>(manifest, path);
        });*/
    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod assets from dir: '{}'", dir.string());
    }
}

template <typename T> static void loadAllXml(Database& db, const Path& path) {
    static const std::set<std::string> exts = {".xml"};

    Log::i(CMP, "Loading xml data from: '{}'", path.string());

    std::set<std::string> keys;

    // Insert or replace all from the directory
    /*db.transaction([&]() {
        iterateDir(path, exts, [&](const Path& file) {
            try {
                T item{};
                Xml::Document(file).getRoot().convert<T>(item);

                const auto key = file.stem().string();
                item.key = key;

                const auto found = db.select<T>("WHERE key = ?", key);
                if (found.empty()) {
                    Log::d("Inserting key into database: '{}'", key);
                    db.insert(item);
                } else {
                    Log::d("Updating key in database: '{}'", key);
                    item.id = found.front().id;
                    db.update(item);
                }

                keys.insert(key);

            } catch (...) {
                EXCEPTION_NESTED("Failed to parse file: '{}'", file.string());
            }
        });

        // Remove items that are not in the folder
        const auto items = db.select<T>();

        for (const auto& item : items) {
            if (const auto it = keys.find(item.key); it == keys.end()) {
                Log::d("Removing key from database: '{}'", item.key);
                db.remove<T>(item.id);
            }
        }
    });*/
}

void ModManager::loadXmlData(AssetManager& assetManager, const Path& dir, Database& db) {
    /*try {
        loadAllXml<Block>(db, dir / Path("blocks"));
        loadAllXml<Asteroid>(db, dir / Path("asteroids"));
    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod data from dir: '{}'", dir.string());
    }*/
}
