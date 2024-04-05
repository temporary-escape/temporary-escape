#pragma once

#include "../../Graphics/Mesh.hpp"
#include "../../Library.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentLines : public Component {
public:
    struct Vertex {
        Vector3 position;
        Color4 color;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)},
            };
        };
    };

    struct Line {
        Vertex a;
        Vertex b;
    };

    static std::vector<Line> createCircle(float radius, const Color4& color);

    ComponentLines() = default;
    explicit ComponentLines(EntityId entity, std::vector<Line> lines);
    COMPONENT_DEFAULTS(ComponentLines);

    void set(std::vector<Line> values) {
        dirty = true;
        lines = std::move(values);
    }

    void append(const std::vector<Line>& values) {
        dirty = true;
        const auto offset = lines.size();
        lines.resize(lines.size() + values.size());
        std::memcpy(lines.data() + offset, values.data(), values.size() * sizeof(Line));
    }

    void add(const Vector3& from, const Vector3& to, const Color4& color) {
        dirty = true;
        lines.push_back({from, color, to, color});
    }

    void add(const Vector2& from, const Vector2& to, const Color4& color) {
        dirty = true;
        lines.push_back({Vector3{from.x, 0.0f, from.y}, color, Vector3{to.x, 0.0f, to.y}, color});
    }

    void recalculate(VulkanRenderer& vulkan);

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

    void setColor(const Color4& value) {
        color = value;
    }

    [[nodiscard]] const Color4& getColor() const {
        return color;
    }

private:
    bool dirty{false};
    std::vector<Line> lines;
    Color4 color{1.0f};
    Mesh mesh;
};
} // namespace Engine
