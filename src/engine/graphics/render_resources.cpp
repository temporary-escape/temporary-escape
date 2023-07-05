#include "render_resources.hpp"
#include "mesh_utils.hpp"

using namespace Engine;

RenderResources::RenderResources(VulkanRenderer& vulkan) {
    meshFullScreenQuad = createFullScreenQuad(vulkan);
    meshPlanet = createPlanetMesh(vulkan);
    meshSkyboxCube = createSkyboxCube(vulkan);
}
