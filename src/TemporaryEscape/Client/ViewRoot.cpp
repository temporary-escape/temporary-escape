#include "ViewRoot.hpp"
#include "Client.hpp"

using namespace Engine;

ViewRoot::ViewRoot(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer,
                   Client& client)
    : config(config), canvas(canvas), client(client), gui(canvas, config, assetManager),
      viewSpace(config, canvas, assetManager, renderer, client, gui),
      viewMap(config, canvas, assetManager, renderer, client, gui), mapActive(false) {
}

void ViewRoot::render(const Vector2i& viewport) {
    const auto t0 = std::chrono::steady_clock::now();

    if (mapActive) {
        viewMap.render(viewport);
    } else {
        viewSpace.render(viewport);
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

    if (mapActive) {
        viewMap.renderGui(viewport);
    } else {
        viewSpace.renderGui(viewport);
    }

    gui.render(viewport);
    canvas.endFrame();

    const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    client.getStats().render.frameTimeMs.store(tDiff.count());
}

void ViewRoot::eventMouseMoved(const Vector2i& pos) {
    if (mapActive) {
        viewMap.eventMouseMoved(pos);
    } else {
        viewSpace.eventMouseMoved(pos);
    }
}

void ViewRoot::eventMousePressed(const Vector2i& pos, MouseButton button) {
    if (mapActive) {
        viewMap.eventMousePressed(pos, button);
    } else {
        viewSpace.eventMousePressed(pos, button);
    }
}

void ViewRoot::eventMouseReleased(const Vector2i& pos, MouseButton button) {
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
