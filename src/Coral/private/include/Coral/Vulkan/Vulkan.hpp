#ifndef CORAL_VULKAN_VULKAN_HPP
#define CORAL_VULKAN_VULKAN_HPP

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>
#include <VkBootstrap.h>
#include <volk.h>

namespace Coral::Vulkan
{
	VkSurfaceKHR createVkSurface(VkInstance instance, void* hwnd);

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_VULKAN_HPP
