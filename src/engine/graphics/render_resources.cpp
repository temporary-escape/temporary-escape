#include "render_resources.hpp"
#include "mesh_utils.hpp"

using namespace Engine;

RenderResources::RenderResources(VulkanRenderer& vulkan) :
    vulkan{vulkan}, defaultSkybox{vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}} {

    meshFullScreenQuad = createFullScreenQuad(vulkan);
    meshPlanet = createPlanetMesh(vulkan);
    meshSkyboxCube = createSkyboxCube(vulkan);
}

RenderResources::~RenderResources() {
    defaultSkybox.dispose(vulkan);
}
