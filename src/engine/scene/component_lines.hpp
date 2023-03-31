#pragma once

#include "../graphics/mesh.hpp"
#include "../library.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentLines : public Component {
public:
    struct Vertex {
        Vector3 position;
        Color4 color;
    };

    struct Line {
        Vertex a;
        Vertex b;
    };

    ComponentLines() = default;
    explicit ComponentLines(std::vector<Line> lines) : lines(std::move(lines)) {
        setDirty(true);
    }
    virtual ~ComponentLines() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentLines);

    void set(std::vector<Line> values) {
        setDirty(true);
        lines = std::move(values);
    }

    void append(const std::vector<Line>& values) {
        setDirty(true);
        const auto offset = lines.size();
        lines.resize(lines.size() + values.size());
        std::memcpy(lines.data() + offset, values.data(), values.size() * sizeof(Line));
    }

    void add(const Vector3& from, const Vector3& to, const Color4& color) {
        setDirty(true);
        lines.push_back({from, color, to, color});
    }

    void add(const Vector2& from, const Vector2& to, const Color4& color) {
        setDirty(true);
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
    std::vector<Line> lines;
    Color4 color{1.0f};
    Mesh mesh;
};
} // namespace Engine
