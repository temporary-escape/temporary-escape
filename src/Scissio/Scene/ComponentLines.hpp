#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "Lines.hpp"

namespace Scissio {
class SCISSIO_API ComponentLines : public Component, public Lines {
public:
    ComponentLines() = default;
    explicit ComponentLines(Object& object);
    virtual ~ComponentLines() = default;

private:
    void rebuildBuffers();

    Mesh mesh{NO_CREATE};
};
} // namespace Scissio
