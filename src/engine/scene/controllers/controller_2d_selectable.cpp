
#include "controller_2d_selectable.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Controller2DSelectable::Controller2DSelectable(entt::registry& reg) : reg{reg} {
    reg.on_construct<Component2DSelectable>().connect<&Controller2DSelectable::onConstruct>(this);
}

Controller2DSelectable::~Controller2DSelectable() {
    reg.on_construct<Component2DSelectable>().disconnect<&Controller2DSelectable::onConstruct>(this);
}

void Controller2DSelectable::update(const float delta) {
}

void Controller2DSelectable::recalculate(VulkanRenderer& vulkan) {
    if (!recreate) {
        return;
    }

    recreate = false;

    std::vector<Point> points;
    components.clear();

    const auto& entities = reg.view<ComponentTransform, Component2DSelectable>().each();
    for (auto&& [handle, transform, component] : entities) {
        const auto pos = transform.getAbsolutePosition();
        const auto width = std::max<float>(component.getSize().x, component.getSize().y);
        points.emplace_back(pos.x, pos.y, pos.z, width);
        components.push_back(&component);
    }

    logger.debug("Recreating {} points", points.size());
    vulkan.dispose(std::move(sboInput));
    vulkan.dispose(std::move(sboOutput));

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Point) * points.size();
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    sboInput = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(sboInput, points.data(), sizeof(Point) * points.size());

    bufferInfo = VulkanDoubleBuffer::CreateInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Output) * points.size();
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    sboOutput = vulkan.createDoubleBuffer(bufferInfo);
}

void Controller2DSelectable::onConstruct(entt::registry& reg, const entt::entity handle) {
    (void)reg;
    recreate = true;
}

void Controller2DSelectable::eventMouseMoved(const Vector2i& pos) {
    const auto point = findNearestPoint(pos);
    if (point) {
        if (hovered && *hovered != *point) {
            components[*hovered]->getOnHoverCallback()(false);
            components[*point]->getOnHoverCallback()(true);
        }
        if (!hovered) {
            components[*point]->getOnHoverCallback()(true);
        }
        hovered = point;
    } else {
        if (hovered) {
            components[*hovered]->getOnHoverCallback()(false);
        }
        hovered = std::nullopt;
    }
}

void Controller2DSelectable::eventMousePressed(const Vector2i& pos, MouseButton button) {
    mousePressed = button;
    mousePressedPos = pos;
}

void Controller2DSelectable::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    if (mousePressed && *mousePressed == button && mousePressedPos == pos && hovered) {
        components[*hovered]->getOnSelectCallback()(button);
    }
}

void Controller2DSelectable::eventMouseScroll(int xscroll, int yscroll) {
}

void Controller2DSelectable::eventKeyPressed(Key key, Modifiers modifiers) {
}

void Controller2DSelectable::eventKeyReleased(Key key, Modifiers modifiers) {
}

void Controller2DSelectable::eventCharTyped(uint32_t code) {
}

std::optional<size_t> Controller2DSelectable::findNearestPoint(const Vector2i& mousePos) {
    if (!sboOutput) {
        return std::nullopt;
    }

    const auto pos = Vector2{mousePos};

    struct Found {
        Output value;
        size_t i;
    };

    std::vector<Found> found;

    auto values = reinterpret_cast<Vector2*>(sboOutput.getPreviousBuffer().getMappedPtr());
    for (size_t i = 0; i < components.size(); i++) {
        const auto size = components[i]->getSize() / 2.0f;
        if (pos.x > values[i].x - size.x && pos.x < values[i].x + size.x && pos.y > values[i].y - size.y &&
            pos.y < values[i].y + size.y) {

            found.push_back({values[i], i});
        }
    }

    if (found.empty()) {
        return std::nullopt;
    }

    std::sort(found.begin(), found.end(), [&](const Found& a, const Found& b) {
        return glm::distance(a.value, pos) < glm::distance(b.value, pos);
    });

    return found.front().i;
}
