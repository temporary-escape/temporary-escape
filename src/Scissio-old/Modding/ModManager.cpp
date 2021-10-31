#include "ModManager.hpp"

#include "../Assets/BasicTexture.hpp"
#include "../Assets/FontFace.hpp"
#include "../Assets/IconAtlas.hpp"
#include "../Assets/Model.hpp"
#include "../Game/Schemas.hpp"
#include "../Utils/Database.hpp"
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

        iterateDir(dir / Path("textures"), {".png"},
                   [&](const Path& path) { assetManager.load<BasicTexture>(manifest, path); });

        iterateDir(dir / Path("images"), {".png"}, [&](const Path& path) { assetManager.load<Image>(manifest, path); });
    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod assets from dir: '{}'", dir.string());
    }
}

template <typename T> static void loadAllXml(Database& db, const Path& path) {
    static const std::set<std::string> exts = {".xml"};

    Log::v("Loading xml data from: '{}'", path.string());

    std::set<std::string> keys;

    // Insert or replace all from the directory
    db.transaction([&]() {
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
    });
}

void ModManager::loadXmlData(const Path& dir, Database& db) {
    try {
        loadAllXml<Block>(db, dir / Path("blocks"));
        loadAllXml<Asteroid>(db, dir / Path("asteroids"));
    } catch (...) {
        EXCEPTION_NESTED("Failed to load mod data from dir: '{}'", dir.string());
    }
}
