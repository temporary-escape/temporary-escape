#pragma once

#include "../config.hpp"
#include "../library.hpp"
#include "../math/vector.hpp"
#include "../utils/image_importer.hpp"
#include "../utils/moveable_copyable.hpp"
#include "../utils/span.hpp"
#include <memory>
#include <optional>
#include <vector>
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

struct ENGINE_API VulkanStageInput {
    std::string name;
    uint32_t location;
    VkFormat format;

    bool operator==(const VulkanStageInput& other) const {
        return name == other.name && location == other.location && format == other.format;
    }

    bool operator!=(const VulkanStageInput& other) const {
        return !(*this == other);
    }
};

struct ENGINE_API VulkanStageUniform {
    std::string name;
    uint32_t binding;
    uint32_t size;

    bool operator==(const VulkanStageUniform& other) const {
        return name == other.name && binding == other.binding && size == other.size;
    }

    bool operator!=(const VulkanStageUniform& other) const {
        return !(*this == other);
    }
};

struct ENGINE_API VulkanStageStorageBuffer {
    std::string name;
    uint32_t binding;

    bool operator==(const VulkanStageStorageBuffer& other) const {
        return name == other.name && binding == other.binding;
    }

    bool operator!=(const VulkanStageStorageBuffer& other) const {
        return !(*this == other);
    }
};

struct ENGINE_API VulkanStagePushMember {
    std::string name;
    VkFormat format;
    size_t offset;
    size_t size;

    bool operator==(const VulkanStagePushMember& other) const {
        return name == other.name && format == other.format && offset == other.offset && size == other.size;
    }

    bool operator!=(const VulkanStagePushMember& other) const {
        return !(*this == other);
    }
};

struct ENGINE_API VulkanStagePushConstants {
    uint32_t size{0};
    std::vector<VulkanStagePushMember> fields;

    bool operator==(const VulkanStagePushConstants& other) const {
        if (size != other.size || fields.size() != other.fields.size()) {
            return false;
        }

        for (size_t i = 0; i < fields.size(); i++) {
            if (fields[i] != other.fields[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const VulkanStagePushConstants& other) const {
        return !(*this == other);
    }
};

struct ENGINE_API VulkanStageSampler {
    std::string name;
    uint32_t binding;
};

struct VulkanVertexLayout {
    uint32_t location;
    VkFormat format;
    size_t offset;
};

using VulkanVertexLayoutMap = std::vector<VulkanVertexLayout>;

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
ENGINE_API bool isDepthFormat(VkFormat format);
} // namespace Engine
