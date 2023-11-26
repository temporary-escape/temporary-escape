#include "ComponentStarFlare.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentStarFlare::ComponentStarFlare(entt::registry& reg, entt::entity handle, TexturePtr texture,
                                       TexturePtr textureLow, TexturePtr textureHigh) :
    Component{reg, handle},
    texture{std::move(texture)},
    textureLow{std::move(textureLow)},
    textureHigh{std::move(textureHigh)} {

    mesh.count = 4;
    mesh.instances = 1;
}
