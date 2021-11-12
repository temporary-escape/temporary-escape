#include "ClientRenderer.hpp"

using namespace Scissio;

ClientRenderer::ClientRenderer(Client& client) : client(client) {
}

ClientRenderer::~ClientRenderer() {
}

void ClientRenderer::render(const Vector2i& viewport, Canvas2D& canvas) {
}
