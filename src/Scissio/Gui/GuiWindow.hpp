#pragma once
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "GuiFlags.hpp"
#include "GuiId.hpp"
#include <string>

namespace Scissio {
class GuiContext;

class SCISSIO_API GuiWindow {
public:
    GuiWindow(GuiContext& ctx, std::string title);
    virtual ~GuiWindow();

    virtual void render() = 0;

    void setPos(const Vector2& pos) {
        this->pos = pos;
    }

    const Vector2& getPos() const {
        return pos;
    }

    void setSize(const Vector2& size) {
        this->size = size;
    }

    const Vector2& getSize() const {
        return size;
    }

    GuiFlags getFlags() const {
        return flags;
    }

    void setFlags(const GuiFlags flags) {
        this->flags = flags;
    }

    const GuiId& getId() const {
        return id;
    }

    const std::string& getTitle() const {
        return title;
    }

    bool isHidden() const {
        return hidden;
    }

    void setHidden(const bool hidden) {
        this->hidden = hidden;
    }

protected:
    GuiContext& ctx;

private:
    GuiId id;
    std::string title;
    Vector2 size{100, 100};
    Vector2 pos{-1, -1};
    bool hidden{false};
    GuiFlags flags{0};
};
} // namespace Scissio
