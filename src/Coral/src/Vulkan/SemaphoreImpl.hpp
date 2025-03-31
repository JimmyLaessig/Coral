#ifndef CORAL_VULKAN_SEMAPHOREIMPL_HPP
#define CORAL_VULKAN_SEMAPHOREIMPL_HPP

#include <Coral/Semaphore.hpp>

#include "ContextImpl.hpp"

namespace Coral::Vulkan
{

class SemaphoreImpl : public Coral::Semaphore
{
public:

	std::optional<Coral::SemaphoreCreationError> init(Coral::Vulkan::ContextImpl& context);

	virtual ~SemaphoreImpl();

	VkSemaphore getVkSemaphore();

	const VkSemaphore getVkSemaphore() const;

private:

	Coral::Vulkan::ContextImpl* mContext{ nullptr };

	VkSemaphore mSemaphore{ VK_NULL_HANDLE };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SEMAPHOREIMPL_HPP