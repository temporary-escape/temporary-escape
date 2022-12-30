#pragma once

#include "../config.hpp"
#include "../library.hpp"
#include "../math/vector.hpp"
#include "../utils/image_importer.hpp"
#include <memory>
#include <optional>
#include <vulkan/vulkan.h>
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>

#define MAX_FRAMES_IN_FLIGHT 2

namespace Engine {
struct ENGINE_API VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily && presentFamily;
    }
};

struct ENGINE_API VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

static ENGINE_API inline const std::vector<const char*> vulkanValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

static ENGINE_API const std::vector<const char*> vulkanDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

class ENGINE_API VulkanDisposable {
public:
    virtual ~VulkanDisposable() = default;
    virtual void destroy() = 0;
};

ENGINE_API VkFormat toVkFormat(const ImageImporter::PixelType pixelType);
ENGINE_API VkDeviceSize getFormatDataSize(const VkFormat format, const VkExtent3D& extent);
} // namespace Engine
