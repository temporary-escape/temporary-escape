#pragma once

#include "../config.hpp"
#include "../library.hpp"
#include "../math/vector.hpp"
#include "../utils/image_importer.hpp"
#include "../utils/span.hpp"
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
    std::optional<uint32_t> computeFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily && presentFamily;
    }
};

struct ENGINE_API VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

static inline const std::vector<const char*> vulkanValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

static const std::vector<const char*> vulkanDeviceExtensions = {
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
