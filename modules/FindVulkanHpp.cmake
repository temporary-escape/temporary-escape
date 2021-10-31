include(FindPackageHandleStandardArgs)

if (NOT TARGET VulkanHpp)
    find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
    mark_as_advanced(FORCE VULKAN_HPP_INCLUDE_DIRS)
    add_library(VulkanHpp INTERFACE IMPORTED)
    set_target_properties(VulkanHpp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${VULKAN_HPP_INCLUDE_DIRS})
endif ()

find_package_handle_standard_args(VulkanHpp DEFAULT_MSG VULKAN_HPP_INCLUDE_DIRS)
