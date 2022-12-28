#pragma once

#include "../config.hpp"
#include "../library.hpp"
#include "../math/vector.hpp"
#include <memory>
#include <optional>
#include <vulkan/vulkan.h>
#define VMA_VULKAN_VERSION 1001000
#include <vk_mem_alloc.h>

namespace Engine {
struct VgQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily && presentFamily;
    }
};

struct VgSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

static inline const std::vector<const char*> vgValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

static const std::vector<const char*> vgDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
} // namespace Engine
