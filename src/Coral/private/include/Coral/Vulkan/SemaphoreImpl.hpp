#ifndef CORAL_VULKAN_SEMAPHOREIMPL_HPP
#define CORAL_VULKAN_SEMAPHOREIMPL_HPP

#include <Coral/Semaphore.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

namespace Coral::Vulkan
{

class SemaphoreImpl : public Coral::Semaphore
	                , public Resource
	                , public std::enable_shared_from_this<SemaphoreImpl>
{
public:
	using Resource::Resource;

	virtual ~SemaphoreImpl();

	std::optional<Coral::SemaphoreCreationError> init();

	VkSemaphore getVkSemaphore();

	const VkSemaphore getVkSemaphore() const;

private:

	VkSemaphore mSemaphore{ VK_NULL_HANDLE };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SEMAPHOREIMPL_HPP
