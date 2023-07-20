include(FindPackageHandleStandardArgs)

find_path(VULKAN_HEADERS_INCLUDE_DIRS "vk_video/vulkan_video_codec_h264std.h")

find_package_handle_standard_args(VulkanHeaders REQUIRED_VARS VULKAN_HEADERS_INCLUDE_DIRS)

if (VulkanHeaders_FOUND)
    mark_as_advanced(VULKAN_HEADERS_INCLUDE_DIRS)
endif ()

if (VulkanHeaders_FOUND AND NOT TARGET VulkanHeaders::VulkanHeaders)
    add_library(VulkanHeaders::VulkanHeaders INTERFACE IMPORTED)
    target_include_directories(VulkanHeaders::VulkanHeaders INTERFACE ${VULKAN_HEADERS_INCLUDE_DIRS})
endif ()
