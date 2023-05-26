#include "component_star_flare.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ComponentStarFlare::ComponentStarFlare(TexturePtr texture, TexturePtr textureLow, TexturePtr textureHigh) :
    texture{std::move(texture)}, textureLow{std::move(textureLow)}, textureHigh{std::move(textureHigh)} {

    mesh.count = 4;
    mesh.instances = 1;
}
