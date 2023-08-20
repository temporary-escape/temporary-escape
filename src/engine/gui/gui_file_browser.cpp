#include "gui_file_browser.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

GuiFileBrowser::GuiFileBrowser(const Config& config) {
    setSize({400.0f, 600.0f});
    setFlags(Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border | Nuklear::WindowFlags::Title);
    setTitle("File Browser");
}

void GuiFileBrowser::drawLayout(Nuklear& nuklear) {
    auto content = nuklear.getContentRegion();
    content.y -= 30.0f * 2.0f + nuklear.getSpacing().y * 2.0f;
    content.y -= 30.0f + nuklear.getSpacing().y;

    nuklear.layoutDynamic(content.y, 1);
    if (nuklear.groupBegin("Files", true)) {
        for (const auto& entry : entries) {
            nuklear.layoutDynamic(0.0f, 1);
            auto selected = entry.filename == filename;
            nuklear.selectable(entry.filename, selected);
            if (selected) {
                filename = entry.filename;
            }
        }

        nuklear.groupEnd();
    }

    nuklear.layoutDynamic(30.0f, 1);
    nuklear.input(filename, 256);

    nuklear.layoutDynamic(30.0f, 4);
    nuklear.layoutSkip();
    if (nuklear.button("Cancel")) {
        setEnabled(false);
    }
    if (nuklear.button(confirmText) && !filename.empty()) {
        setEnabled(false);
        if (callback) {
            callback(getPath());
        }
    }
    nuklear.layoutSkip();
}

void GuiFileBrowser::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - getSize().y / 2});
}

void GuiFileBrowser::setFolder(const Path& dir, const std::string& ext) {
    directory = dir;
    extension = ext;
    entries.clear();

    const auto iterator = std::filesystem::directory_iterator{dir};
    for (const auto& entry : iterator) {
        const auto& path = entry.path();
        if (path.extension() != extension) {
            continue;
        }

        entries.emplace_back(Entry{path, path.stem().string()});
    }
}

Path GuiFileBrowser::getPath() const {
    if (filename.empty()) {
        return "";
    }
    return directory / fmt::format("{}{}", filename, extension);
}
