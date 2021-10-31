#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "Lines.hpp"

namespace Scissio {
class SCISSIO_API ComponentLines : public Component, public Lines {
public:
    static constexpr ComponentType Type = 5;

    ComponentLines();
    explicit ComponentLines(Object& object);
    virtual ~ComponentLines() = default;

    void render(Shader& shader);

private:
    void rebuildBuffers();

    Mesh mesh;
};
} // namespace Scissio
