#include "render_resources.hpp"
#include "../math/utils.hpp"
#include "mesh_utils.hpp"

using namespace Engine;

static const size_t ssaoKernelSize = 16;
static const size_t ssaoKernelMax = 64;

static VulkanTexture createTextureOfColor(VulkanRenderer& vulkan, const Color4& color, const int layers) {
    std::unique_ptr<uint8_t[]> pixels(new uint8_t[4 * 4 * 4]);

    for (size_t i = 0; i < 4 * 4 * 4; i += 4) {
        pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
        pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
        pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
        pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {4, 4, 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = layers;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.view.viewType = layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = layers;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    auto texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    for (int i = 0; i < layers; i++) {
        vulkan.copyDataToImage(texture, 0, {0, 0}, i, {4, 4}, pixels.get());
    }
    vulkan.transitionImageLayout(
        texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return texture;
}

RenderResources::RenderResources(VulkanRenderer& vulkan) :
    vulkan{vulkan}, defaultSkybox{vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}} {

    meshFullScreenQuad = createFullScreenQuad(vulkan);
    meshPlanet = createPlanetMesh(vulkan);
    meshSkyboxCube = createSkyboxCube(vulkan);
    meshBullet = createBulletMesh(vulkan);
    createSsaoNoise();
    createSsaoSamples();
    defaultSSAO = createTextureOfColor(vulkan, Color4{1.0f, 1.0f, 1.0f, 1.0f}, 1);
    defaultShadow = createTextureOfColor(vulkan, Color4{1.0f, 1.0f, 1.0f, 1.0f}, 4);
    defaultBloom = createTextureOfColor(vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}, 1);
}

RenderResources::~RenderResources() {
    defaultSkybox.dispose(vulkan);
}

void RenderResources::createSsaoNoise() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator{rd()};

    std::vector<Vector4> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        Vector4 noise{
            // rotate around z-axis (in tangent space)
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f,
            0.0f,
        };
        ssaoNoise.push_back(noise);
    }

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {4, 4, 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    ssaoSamples.noise = vulkan.createTexture(textureInfo);
    vulkan.transitionImageLayout(ssaoSamples.noise, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan.copyDataToImage(ssaoSamples.noise, 0, {0, 0}, 0, {4, 4}, ssaoNoise.data());
    vulkan.transitionImageLayout(
        ssaoSamples.noise, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void RenderResources::createSsaoSamples() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::random_device rd;
    std::default_random_engine generator{rd()};

    std::array<Vector4, ssaoKernelMax> weights{};

    for (unsigned int i = 0; i < ssaoKernelSize; ++i) {
        Vector3 sample(
            randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / static_cast<float>(ssaoKernelSize);

        // scale samples s.t. they're more aligned to center of kernel
        scale = ::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        weights[i] = Vector4{sample, 1.0f};
    }

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vector4) * weights.size();
    bufferInfo.usage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    ssaoSamples.ubo = vulkan.createBuffer(bufferInfo);
    vulkan.copyDataToBuffer(ssaoSamples.ubo, weights.data(), bufferInfo.size);
}
