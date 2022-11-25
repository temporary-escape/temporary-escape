#pragma once

#include "../library.hpp"
#include "vez/VEZ.h"
#include <type_traits>
#include <vector>

namespace Engine {
class VulkanVertexInputFormat {
public:
    enum class Format {
        Float = VkFormat::VK_FORMAT_R32_SFLOAT,
        Vec2 = VkFormat::VK_FORMAT_R32G32_SFLOAT,
        Vec3 = VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
        Vec4 = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,
    };

    struct Attribute {
        uint32_t location;
        uint32_t binding;
        Format format;
    };

    struct Binding {
        uint32_t index{0};
        const std::vector<Attribute>& attributes;
    };

    NON_COPYABLE(VulkanVertexInputFormat);

    VulkanVertexInputFormat() = default;
    explicit VulkanVertexInputFormat(VkDevice device, const std::vector<Binding>& bindings);
    ~VulkanVertexInputFormat();
    VulkanVertexInputFormat(VulkanVertexInputFormat&& other) noexcept;
    VulkanVertexInputFormat& operator=(VulkanVertexInputFormat&& other) noexcept;
    void swap(VulkanVertexInputFormat& other) noexcept;
    void reset();

    [[nodiscard]] VezVertexInputFormat& getHandle() {
        return format;
    }

    [[nodiscard]] const VezVertexInputFormat& getHandle() const {
        return format;
    }

    [[nodiscard]] operator bool() const {
        return format != VK_NULL_HANDLE;
    }

private:
    VkDevice device{VK_NULL_HANDLE};
    VezVertexInputFormat format{VK_NULL_HANDLE};
};

static_assert(std::is_move_constructible<VulkanVertexInputFormat>::value,
              "VulkanVertexInputFormat must be move constructible");
static_assert(std::is_move_assignable<VulkanVertexInputFormat>::value,
              "VulkanVertexInputFormat must be move assignable");
} // namespace Engine
