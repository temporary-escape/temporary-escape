#pragma once

#include "GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiFileBrowser : public GuiWindow {
public:
    explicit GuiFileBrowser(const Config& config);
    ~GuiFileBrowser() override = default;

    void setFolder(const Path& dir, const std::string& ext);
    void setConfirmText(std::string value) {
        confirmText = std::move(value);
    }
    template <typename Fn> void setConfirmCallback(Fn&& fn) {
        callback = std::forward<Fn>(fn);
    }
    Path getPath() const;

private:
    struct Entry {
        Path path;
        std::string filename;
    };

    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    std::function<void(Path)> callback;
    Path directory;
    std::string confirmText;
    std::vector<Entry> entries;
    std::string filename;
    std::string extension;
};
} // namespace Engine
