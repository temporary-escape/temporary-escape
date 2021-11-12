#pragma once

#include "../Graphics/Canvas2D.hpp"
#include "Client.hpp"

namespace Scissio {
class ClientRenderer {
public:
    ClientRenderer(Client& client);
    ~ClientRenderer();

    void render(const Vector2i& viewport, Canvas2D& canvas);

private:
    Client& client;
};
} // namespace Scissio