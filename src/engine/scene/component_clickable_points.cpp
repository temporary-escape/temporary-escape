#include "component_clickable_points.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void ComponentClickablePoints::add(const Vector3& pos) {
    setDirty(true);
    points.emplace_back(pos, 1.0f);
}

void ComponentClickablePoints::clear() {
    setDirty(true);
    points.clear();
    points.shrink_to_fit();
}

void ComponentClickablePoints::recalculate(VulkanRenderer& vulkan) {
    if (!isDirty()) {
        return;
    }

    setDirty(false);

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

Vector2 ComponentClickablePoints::getOutputPoint(size_t idx) const {
    auto values = reinterpret_cast<Vector2*>(sboOutput.getPreviousBuffer().getMappedPtr());
    return values[idx];
}

std::optional<size_t> ComponentClickablePoints::findNearestPoint(const Vector2i& mousePos) {
    const auto pos = Vector2{mousePos};

    struct Found {
        Output value;
        size_t i;
    };

    std::vector<Found> found;

    auto values = reinterpret_cast<Vector2*>(sboOutput.getPreviousBuffer().getMappedPtr());
    for (size_t i = 0; i < points.size(); i++) {
        if (pos.x > values[i].x - size.x / 2.0f && pos.x < values[i].x + size.x / 2.0f &&
            pos.y > values[i].y - size.y / 2.0f && pos.y < values[i].y + size.y / 2.0f) {

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

void ComponentClickablePoints::eventMouseMoved(const Vector2i& pos) {
    const auto point = findNearestPoint(pos);
    if (point) {
        blur = true;
        if (onHoverCallback) {
            onHoverCallback(*point);
        }
    } else if (blur) {
        blur = false;
        if (onBlurCallback) {
            onBlurCallback();
        }
    }
}

void ComponentClickablePoints::eventMousePressed(const Vector2i& pos, MouseButton button) {
    const auto point = findNearestPoint(pos);
    if (point) {
        if (onClickCallback) {
            onClickCallback(*point, true, button);
        }
    }
}

void ComponentClickablePoints::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    const auto point = findNearestPoint(pos);
    if (point) {
        if (onClickCallback) {
            onClickCallback(*point, false, button);
        }
    }
}

void ComponentClickablePoints::eventMouseScroll(int xscroll, int yscroll) {
}

void ComponentClickablePoints::eventKeyPressed(Key key, Modifiers modifiers) {
}

void ComponentClickablePoints::eventKeyReleased(Key key, Modifiers modifiers) {
}

void ComponentClickablePoints::eventCharTyped(uint32_t code) {
}
