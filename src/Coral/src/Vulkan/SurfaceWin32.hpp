#ifndef CORAL_VULKAN_SURFACEWIN32_HPP
#define CORAL_VULKAN_SURFACEWIN32_HPP

#include "VulkanWrapper.hpp"

namespace Coral::Vulkan
{

VkSurfaceKHR createVkSurface(VkInstance instance, void* hwnd);

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SURFACEWIN32_HPP
