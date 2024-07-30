#include "RenderResources.hpp"
#include "../File/Ktx2FileReader.hpp"
#include "../Math/Utils.hpp"
#include "MeshUtils.hpp"
#include <brdf.ktx2.h>
#include <palette.ktx2.h>
#include <skybox_star.ktx2.h>
#include <space_dust.ktx2.h>

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

RenderResources::RenderResources(VulkanRenderer& vulkan, const VulkanBuffer& blockMaterials,
                                 const VulkanBuffer& particlesTypes, const MaterialTextures& materialTextures,
                                 const FontFamily& font, const int fontSize) :
    vulkan{vulkan},
    blockMaterials{blockMaterials},
    particlesTypes{particlesTypes},
    materialTextures{materialTextures},
    textTacticalOverlay{font},
    defaultSkybox{vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}} {

    meshFullScreenQuad = createFullScreenQuad(vulkan);
    meshPlanet = createPlanetMesh(vulkan);
    meshSkyboxCube = createSkyboxCube(vulkan);
    meshBullet = createBulletMesh(vulkan);
    meshOrbit = createCircleMesh(vulkan);
    meshSpaceDust = createSpaceDustMesh(vulkan);
    meshLineForward = createLineForwardMesh(vulkan);
    meshTacticalOverlay = createTacticalOverlayMesh(vulkan);
    createSsaoNoise();
    createSsaoSamples();
    createPalette();
    createBrdf();
    createSkyboxStar();
    createSpaceDust();
    createTextTacticalOverlay(fontSize);
    defaultSSAO = createTextureOfColor(vulkan, Color4{1.0f, 1.0f, 1.0f, 1.0f}, 1);
    defaultShadow = createTextureOfColor(vulkan, Color4{1.0f, 1.0f, 1.0f, 1.0f}, 4);
    defaultBloom = createTextureOfColor(vulkan, Color4{0.0f, 0.0f, 0.0f, 1.0f}, 1);

    descriptorPool = VulkanDescriptorPool{vulkan};

    {
        std::array<VkDescriptorSetLayoutBinding, 8> bindings{};
        auto binding = bindings.begin();

        binding->binding = 0;
        binding->descriptorCount = 1;
        binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding->pImmutableSamplers = nullptr;
        binding->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                              VK_SHADER_STAGE_COMPUTE_BIT;
        binding++;

        for (size_t i = 1; i <= 7; i++) {
            binding->binding = i;
            binding->descriptorCount = 1;
            binding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding->pImmutableSamplers = nullptr;
            binding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding++;
        }

        blockMaterialsDescriptorSetLayout = VulkanDescriptorSetLayout{vulkan, bindings};

        blockMaterialsDescriptorSet =
            VulkanDescriptorSet{vulkan.getDevice(), descriptorPool, blockMaterialsDescriptorSetLayout};

        std::array<VulkanBufferBinding, 1> uniforms{};
        std::array<VulkanTextureBinding, 7> textures{};
        uniforms[0] = {0, &blockMaterials};
        textures[0] = {1, &materialTextures.get(TextureUsage::Diffuse).getTexture()};
        textures[1] = {2, &materialTextures.get(TextureUsage::Emissive).getTexture()};
        textures[2] = {3, &materialTextures.get(TextureUsage::Normal).getTexture()};
        textures[3] = {4, &materialTextures.get(TextureUsage::AmbientOcclusion).getTexture()};
        textures[4] = {5, &materialTextures.get(TextureUsage::MetallicRoughness).getTexture()};
        textures[5] = {6, &materialTextures.get(TextureUsage::Mask).getTexture()};
        textures[6] = {7, &palette};
        blockMaterialsDescriptorSet.bind(uniforms, textures, {});
    }
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

static VulkanTexture::CreateInfo createCreateInfo(Ktx2FileReader& image) {
    const auto& size = image.getSize();

    VkExtent3D extent{
        static_cast<uint32_t>(size.x),
        static_cast<uint32_t>(size.y),
        1,
    };

    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = extent;
    textureInfo.image.mipLevels = image.getMipMapsCount();
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.format = textureInfo.image.format;
    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = textureInfo.image.mipLevels;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;

    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.maxLod = static_cast<float>(textureInfo.image.mipLevels);

    return textureInfo;
}

static VulkanTexture createTexture(VulkanRenderer& vulkan, Ktx2FileReader& image,
                                   VulkanTexture::CreateInfo& textureInfo) {
    if (image.needsTranscoding()) {
        image.transcode(VulkanCompressionType::None, TextureCompressionTarget::RGBA);
    }
    image.readData();

    textureInfo.image.format = image.getFormat();
    VulkanTexture texture = vulkan.createTexture(textureInfo);

    vulkan.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    for (auto i = 0; i < textureInfo.image.mipLevels; i++) {
        const auto& chunk = image.getData(i, 0);
        vulkan.copyDataToImage(texture, i, {0, 0}, 0, chunk.size, chunk.pixels.data(), chunk.pixels.size());
    }

    vulkan.transitionImageLayout(
        texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return texture;
}

void RenderResources::createPalette() {
    Ktx2FileReader image{Embed::palette_ktx2};

    if (image.getSize().y != 1) {
        EXCEPTION("Expected palette texture to be 1D in size but instead is: {}", image.getSize());
    }

    auto textureInfo = createCreateInfo(image);
    textureInfo.image.imageType = VK_IMAGE_TYPE_1D;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_1D;
    textureInfo.sampler.magFilter = VK_FILTER_NEAREST;
    textureInfo.sampler.minFilter = VK_FILTER_NEAREST;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    palette = createTexture(vulkan, image, textureInfo);
}

void RenderResources::createBrdf() {
    Ktx2FileReader image{Embed::brdf_ktx2};

    auto textureInfo = createCreateInfo(image);
    textureInfo.image.mipLevels = 1;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.sampler.maxLod = 1.0f;
    brdf = createTexture(vulkan, image, textureInfo);
}

void RenderResources::createSkyboxStar() {
    Ktx2FileReader image{Embed::skybox_star_ktx2};

    auto textureInfo = createCreateInfo(image);
    skyboxStar = createTexture(vulkan, image, textureInfo);
}

void RenderResources::createSpaceDust() {
    Ktx2FileReader image{Embed::space_dust_ktx2};

    auto textureInfo = createCreateInfo(image);
    spaceDust = createTexture(vulkan, image, textureInfo);
}

void RenderResources::createTextTacticalOverlay(const int fontSize) {
    static std::array<float, 8> ranges = {
        1.0f,
        2.0f,
        5.0f,
        10.0f,
        20.0f,
        30.0f,
        40.0f,
        50.0f,
    };

    static std::array<float, 4> angles = {
        0.0f,
        90.0f,
        180.0f,
        270.0f,
    };

    for (const auto range : ranges) {
        for (const auto angle : angles) {
            const auto pos = glm::rotateY(Vector3{range * 1000.0f, 0.0f, 0.0f}, glm::radians(angle));
            textTacticalOverlay.add(pos, fmt::format("{}km", static_cast<int>(range)), fontSize);
        }
    }

    textTacticalOverlay.recalculate(vulkan);
}
