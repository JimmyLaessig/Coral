#ifndef CORAL_VULKAN_SEMAPHOREIMPL_HPP
#define CORAL_VULKAN_SEMAPHOREIMPL_HPP

#include <Coral/SemaphoreBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>

namespace Coral::Vulkan
{

class SemaphoreImpl : public Coral::SemaphoreBase
{
public:
	using SemaphoreBase::SemaphoreBase;

	virtual ~SemaphoreImpl();

	std::optional<Coral::SemaphoreCreationError> init();

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

	VkSemaphore getVkSemaphore();

	const VkSemaphore getVkSemaphore() const;

private:

	VkSemaphore mSemaphore{ VK_NULL_HANDLE };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SEMAPHOREIMPL_HPP