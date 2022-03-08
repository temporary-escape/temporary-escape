#include "ViewRoot.hpp"
#include "Client.hpp"

using namespace Engine;

ViewRoot::ViewRoot(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                   Client& client)
    : config(config), canvas(canvas), client(client), gui(canvas, config, assetManager), widgets(gui),
      viewSpace(config, canvas, assetManager, renderer, client, widgets),
      viewMap(config, canvas, assetManager, renderer, client, widgets), mapActive(false) {
}

void ViewRoot::render(const Vector2i& viewport) {
    const auto t0 = std::chrono::steady_clock::now();

    try {
        if (mapActive) {
            viewMap.render(viewport);
        } else {
            viewSpace.render(viewport);
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to render the scene");
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    canvas.beginFrame(viewport);
    gui.reset();

    widgets.update(viewport);

    try {
        if (mapActive) {
            viewMap.renderGui(viewport);
        } else {
            viewSpace.renderGui(viewport);
        }

        gui.render(viewport);

    } catch (...) {
        EXCEPTION_NESTED("Failed to render the gui");
    }

    canvas.endFrame();

    const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    client.getStats().render.frameTimeMs.store(tDiff.count());
}

void ViewRoot::eventMouseMoved(const Vector2i& pos) {
    gui.mouseMoveEvent(pos);
    if (mapActive) {
        viewMap.eventMouseMoved(pos);
    } else {
        viewSpace.eventMouseMoved(pos);
    }
}

void ViewRoot::eventMousePressed(const Vector2i& pos, MouseButton button) {
    gui.mousePressEvent(pos, button);
    if (mapActive) {
        viewMap.eventMousePressed(pos, button);
    } else {
        viewSpace.eventMousePressed(pos, button);
    }
}

void ViewRoot::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    gui.mouseReleaseEvent(pos, button);
    if (mapActive) {
        viewMap.eventMouseReleased(pos, button);
    } else {
        viewSpace.eventMouseReleased(pos, button);
    }
}

void ViewRoot::eventMouseScroll(int xscroll, int yscroll) {
    if (mapActive) {
        viewMap.eventMouseScroll(xscroll, yscroll);
    } else {
        viewSpace.eventMouseScroll(xscroll, yscroll);
    }
}

void ViewRoot::eventKeyPressed(Key key, Modifiers modifiers) {
    if (mapActive) {
        if (key == Key::LetterM) {
            mapActive = false;
        } else {
            viewMap.eventKeyPressed(key, modifiers);
        }
    } else {
        if (key == Key::LetterM) {
            mapActive = true;
            viewMap.load();
        } else {
            viewSpace.eventKeyPressed(key, modifiers);
        }
    }
}

void ViewRoot::eventKeyReleased(Key key, Modifiers modifiers) {
    if (mapActive) {
        viewMap.eventKeyReleased(key, modifiers);
    } else {
        viewSpace.eventKeyReleased(key, modifiers);
    }
}
