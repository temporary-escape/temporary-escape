#include "render_resources.hpp"
#include "../scene/components/component_point_cloud.hpp"
#include "mesh_utils.hpp"

using namespace Engine;

static const float PI = static_cast<float>(std::atan(1) * 4);

RenderResources::RenderResources(VulkanRenderer& vulkan) {
    meshFullScreenQuad = createFullScreenQuad(vulkan);
    meshPlanet = createPlanetMesh(vulkan);
    meshSkyboxCube = createSkyboxCube(vulkan);
}
