#include "SurfaceWin32.hpp"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#undef min

namespace Coral::Vulkan
{

VkSurfaceKHR
createVkSurface(VkInstance instance, void* hwnd)
{
	auto vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (!vkCreateWin32SurfaceKHR)
	{
		//	"Win32: Vulkan instance missing VK_KHR_win32_surface extension");
		return VK_NULL_HANDLE;
	}
	HWND handle			= static_cast<HWND>(hwnd);
	HINSTANCE hinstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(handle, GWLP_HINSTANCE));

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceCreateInfo.hwnd		= handle;
	surfaceCreateInfo.hinstance = hinstance;

	VkSurfaceKHR surface{ VK_NULL_HANDLE };
	if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	return surface;
}

} // namespace Coral::Vulkan