#include "View.hpp"
#include "../Assets/AssetManager.hpp"
#include "Client.hpp"

#define CMP "View"

using namespace Engine;

/*View::View(const Config& config, Canvas2D& canvas, AssetManager& assetManager, Renderer& renderer, Client& client)
    : config(config), canvas(canvas), assetManager(assetManager), renderer(renderer), client(client),
      gui(canvas, config, assetManager) {

    galaxyMap.starImage = assetManager.find<AssetImage>("star_flare");
    galaxyMap.markerImage = assetManager.find<AssetImage>("icon_position_marker");
    systemMap.planetImage = assetManager.find<AssetImage>("icon_ringed_planet");
    systemMap.starImage = assetManager.find<AssetImage>("icon_sun");
    systemMap.markerImage = assetManager.find<AssetImage>("icon_position_marker");

    fontFaceRegular = assetManager.find<AssetFontFamily>("iosevka-aile")->get("regular");
}

void View::render(const Vector2i& viewport) {
    const auto t0 = std::chrono::steady_clock::now();

    const auto mapActive = galaxyMap.active || systemMap.active;

    if (auto scene = client.getScene(); scene != nullptr && !mapActive) {
        auto& componentSystemCamera = scene->getComponentSystem<ComponentCamera>();
        auto front = componentSystemCamera.begin();
        if (front != componentSystemCamera.end()) {
            auto component = *front;

            glm::mat4x4 transform{1.0f};
            transform = glm::rotate(transform, glm::radians(camera.rotation.x), Vector3{0.0f, 1.0f, 0.0f});
            transform = glm::rotate(transform, glm::radians(camera.rotation.y), Vector3{1.0f, 0.0f, 0.0f});

            const auto eyes = camera.pos + Vector3(transform * Vector4(0.0f, 0.0f, camera.zoom, 1.0f));
            component->lookAt(eyes, camera.pos);
        }

        renderer.render(viewport, *scene);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    canvas.beginFrame(viewport);

    gui.reset();

    if (galaxyMap.active) {
        renderGalaxyMap(viewport);
    } else if (systemMap.active) {
        renderSystemMap(viewport);
    }

    // widgets.debugStats.render();

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    gui.render(viewport);
    canvas.endFrame();

    const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    client.getStats().render.frameTimeMs.store(tDiff.count());
}

void View::renderGalaxyMap(const Vector2i& viewport) {
    const auto middle = Vector2{viewport / 2};
    const auto starSize = Vector2{32.0f, 32.0f};
    const auto markerSize = Vector2{32.0f, 32.0f};

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    static const auto colorConnectionInRegion = GuiColors::secondary * alpha(0.4f);
    static const auto colorConnectionOutRegion = Color4{1.0f, 1.0f, 1.0f, 0.1f};

    const auto& location = galaxyMap.location;
    const auto& currentSystem = galaxyMap.systems.at(location.systemId);
    const auto& currentRegion = galaxyMap.regions.at(currentSystem.regionId);

    // Systems connections inside of the region
    canvas.beginPath();
    canvas.strokeWidth(2.0f);
    canvas.strokeColor(colorConnectionInRegion);
    for (const auto& [_, system] : galaxyMap.systems) {
        for (const auto& otherId : system.connections) {
            const auto& other = galaxyMap.systems.at(otherId);
            if (system.regionId == currentSystem.regionId || other.regionId == currentSystem.regionId) {
                canvas.moveTo(galaxyMap.camera.worldToScreen(system.pos));
                canvas.lineTo(galaxyMap.camera.worldToScreen(other.pos));
            }
        }
    }
    canvas.stroke();
    canvas.closePath();

    // System connections outside of the reion
    canvas.beginPath();
    canvas.strokeWidth(2.0f);
    canvas.strokeColor(colorConnectionOutRegion);
    for (const auto& [_, system] : galaxyMap.systems) {
        for (const auto& otherId : system.connections) {
            const auto& other = galaxyMap.systems.at(otherId);
            if (system.regionId != currentSystem.regionId && other.regionId != currentSystem.regionId) {
                canvas.moveTo(galaxyMap.camera.worldToScreen(system.pos));
                canvas.lineTo(galaxyMap.camera.worldToScreen(other.pos));
            }
        }
    }
    canvas.stroke();
    canvas.closePath();

    static const auto colorStarInRegion = Color4{1.0f, 1.0f, 1.0f, 1.0f};
    static const auto colorStarOutRegion = Color4{1.0f, 1.0f, 1.0f, 0.7f};

    // System stars
    canvas.fillColor(Color4{1.0f});
    for (const auto& [_, system] : galaxyMap.systems) {
        const Color4& color = (system.regionId == currentSystem.regionId) ? colorStarInRegion : colorStarOutRegion;

        canvas.beginPath();
        canvas.rectImage(galaxyMap.camera.worldToScreen(system.pos) - (starSize / 2.0f), starSize,
                         galaxyMap.starImage->getImage(), color);
        canvas.fill();
        canvas.closePath();
    }

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    // Current location marker
    const auto offset = Vector2{markerSize.x / -2.0f, -markerSize.y};
    canvas.beginPath();
    canvas.rectImage(galaxyMap.camera.worldToScreen(currentSystem.pos) + offset, markerSize,
                     galaxyMap.markerImage->getImage(), GuiColors::primary);
    canvas.fill();
    canvas.closePath();

    // Current location left top text
    canvas.fontFace(fontFaceRegular->getHandle());
    canvas.fontSize(48.0f);
    const auto box = canvas.textBounds(currentSystem.name);
    canvas.beginPath();
    canvas.fillColor(GuiColors::backgroundTransparent);
    canvas.rect({30.0f, 50.0f}, Vector2{box.x + 54.0f, 200.0f});
    canvas.fill();
    canvas.closePath();

    canvas.beginPath();
    canvas.rectImage({30.0f, 50.0f + 4.0f}, Vector2{48.0f}, galaxyMap.markerImage->getImage(), GuiColors::primary);
    canvas.fill();
    canvas.closePath();

    canvas.fontSize(48.0f);
    canvas.fillColor(GuiColors::primary);
    canvas.text({86.0f, 50.0f + 48.0f}, currentSystem.name);

    canvas.fontSize(24.0f);
    canvas.fillColor(GuiColors::text);
    canvas.text({90.0f, 50.0f + 48.0f + 28.0f}, fmt::format("Region: {}", currentRegion.name));
}

void View::renderSystemMap(const Vector2i& viewport) {
    const auto middle = Vector2{viewport / 2};
    const auto planetSize = Vector2{32.0f, 32.0f};

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    canvas.fillColor(Color4{1.0f});

    canvas.beginPath();
    canvas.rectImage(middle - planetSize / 2.0f, planetSize, systemMap.starImage->getImage());
    canvas.fill();
    canvas.closePath();

    for (const auto& [id, planet] : systemMap.planets) {
        if (planet.isMoon) {

        } else {
            const auto radius = glm::length(planet.pos);
            canvas.beginPath();
            canvas.strokeColor(Color4{1.0f, 1.0f, 1.0f, 0.25f});
            canvas.strokeWidth(2.0f);
            canvas.moveTo(middle + Vector2{radius, 0} * scale);
            const auto angle = 360.0f / 128.0f;
            for (auto i = 1; i <= 128; i++) {
                auto pos = Vector2{radius, 0} * scale;
                pos = glm::rotate(pos, glm::radians(i * angle));
                canvas.lineTo(middle + pos);
            }
            canvas.stroke();
            canvas.closePath();
        }

        canvas.beginPath();
        canvas.rectImage(middle + planet.pos * scale - planetSize / 2.0f, planetSize,
                         systemMap.planetImage->getImage());
        canvas.fill();
        canvas.closePath();
    }
}

void View::loadGalaxyMap() {
    MessageFetchCurrentLocation reqLoc;

    client.fetch(reqLoc, [this](PlayerLocationData location) {
        MessageFetchGalaxyRegions reqRegions;
        reqRegions.galaxyId = location.galaxyId;
        galaxyMap.location = location;

        client.fetch(reqRegions, [this, location](std::vector<RegionData> items) {
            galaxyMap.regions.clear();
            for (const auto& item : items) {
                galaxyMap.regions.insert(std::make_pair(item.id, item));
            }

            MessageFetchGalaxySystems reqSystems;
            reqSystems.galaxyId = location.galaxyId;

            client.fetch(reqSystems, [this](std::vector<SystemData> items) {
                galaxyMap.systems.clear();
                for (const auto& item : items) {
                    galaxyMap.systems.insert(std::make_pair(item.id, item));
                }
                galaxyMap.active = true;
            });
        });
    });
}

void View::loadSystemMap() {
    MessageFetchCurrentLocation reqLoc;

    client.fetch(reqLoc, [this](PlayerLocationData location) {
        MessageFetchSystemPlanets reqPlanets;
        reqPlanets.galaxyId = location.galaxyId;
        reqPlanets.systemId = location.systemId;

        client.fetch(reqPlanets, [this, location](std::vector<SectorPlanetData> items) {
            systemMap.planets.clear();
            for (const auto& item : items) {
                systemMap.planets.insert(std::make_pair(item.id, item));
            }

            systemMap.active = true;
        });
    });
}

void View::eventMouseMoved(const Vector2i& pos) {
    if (galaxyMap.active) {
        galaxyMap.camera.eventMouseMoved(pos);
    } else if (systemMap.active) {
        systemMap.camera.eventMouseMoved(pos);
    } else if (camera.rotate) {
        camera.rotation += (camera.mousePosOld - Vector2(pos)) * 0.2f;
        while (camera.rotation.x > 360.0f) {
            camera.rotation.x -= 360.0f;
        }
        while (camera.rotation.x < 0.0f) {
            camera.rotation.x += 360.0f;
        }
        camera.rotation.y = glm::clamp(camera.rotation.y, -89.0f, 89.0f);
        camera.mousePosOld = pos;
    }
}

void View::eventMousePressed(const Vector2i& pos, MouseButton button) {
    camera.mousePosOld = pos;

    if (galaxyMap.active) {
        galaxyMap.camera.eventMousePressed(pos, button);
    } else if (systemMap.active) {
        systemMap.camera.eventMousePressed(pos, button);
    } else {
        camera.rotate |= button == MouseButton::Right;
    }
}

void View::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    camera.mousePosOld = pos;
    camera.rotate &= !(button == MouseButton::Right);
    galaxyMap.camera.eventMouseReleased(pos, button);
    systemMap.camera.eventMouseReleased(pos, button);
}

void View::eventMouseScroll(int xscroll, int yscroll) {
    if (galaxyMap.active) {
        galaxyMap.camera.eventMouseScroll(xscroll, yscroll);
    } else if (systemMap.active) {
        systemMap.camera.eventMouseScroll(xscroll, yscroll);
    } else {
        const auto factor = map(camera.zoom, 1.0f, 50.0f, 0.2f, 5.0f);
        camera.zoom += static_cast<float>(-yscroll) * factor;

        if (camera.zoom < 1.0f) {
            camera.zoom = 1.0f;
        } else if (camera.zoom > 50.0f) {
            camera.zoom = 50.0f;
        }
    }
}

void View::eventKeyPressed(Key key, Modifiers modifiers) {
    if (key == Key::LetterM) {
        if (galaxyMap.active) {
            galaxyMap.active = false;
            systemMap.active = false;
        } else {
            loadGalaxyMap();
        }
    }

    if (key == Key::LetterN) {
        if (systemMap.active) {
            galaxyMap.active = false;
            systemMap.active = false;
        } else {
            loadSystemMap();
        }
    }
}

void View::eventKeyReleased(Key key, Modifiers modifiers) {
}

View::OrthoCamera::OrthoCamera() : move(false), zoom(1.0f), zoomMin(1.0f), zoomMax(50.0f) {
    moveTo(Vector2{0.0f});
}

void View::OrthoCamera::moveTo(const Vector2& pos) {
    position = pos;
    transform = Matrix3{1.0f};
    // TODO:
    transform = glm::translate(transform, Vector3{position.x, position.y, 0.0f});
    transform = glm::scale(transform, Vector3{zoom});
}

void View::OrthoCamera::eventMouseMoved(const Vector2i& pos) {
    if (move) {
        const auto from = screenToWorld(mousePosOld);
        const auto to = screenToWorld(pos);
        const auto diff = to - from;
        moveTo(position + diff);
        mousePosOld = pos;
    }
}

void View::OrthoCamera::eventMousePressed(const Vector2i& pos, MouseButton button) {
    mousePosOld = pos;
    move |= button == MouseButton::Right;
}

void View::OrthoCamera::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    mousePosOld = pos;
    move &= !(button == MouseButton::Right);
}

void View::OrthoCamera::eventMouseScroll(int xscroll, int yscroll) {
    const auto factor = map(zoom, zoomMin, zoomMax, 1.0f, 0.01f);
    zoom += static_cast<float>(yscroll) * factor;

    if (zoom < zoomMin) {
        zoom = zoomMin;
    } else if (zoom > zoomMax) {
        zoom = zoomMax;
    }

    moveTo(position);
}

Vector2 View::OrthoCamera::worldToScreen(const Vector2& other) {
    const auto res = transform * Vector4{other.x, other.y, 0.0f, 1.0f};
    return {res.x, res.y};
}

Vector2 View::OrthoCamera::screenToWorld(const Vector2& other) {
    const auto inverted = glm::inverse(transform);
    const auto res = inverted * Vector4{other.x, other.y, 0.0f, 1.0f};
    return {res.x * zoom, res.y * zoom};
}

void View::OrthoCamera::stopMove() {
    move = false;
}*/
