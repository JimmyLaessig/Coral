#ifndef CORAL_VULKAN_FENCEIMPL_HPP
#define CORAL_VULKAN_FENCEIMPL_HPP

#include <Coral/Fence.hpp>

#include "ContextImpl.hpp"


namespace Coral::Vulkan
{

class FenceImpl: public Coral::Fence
{
public:

	virtual ~FenceImpl();

	std::optional<Coral::FenceCreationError> init(ContextImpl& context);

	VkFence getVkFence();

	bool wait() override;

	void reset() override;

private:

	ContextImpl* mContext{ nullptr };

	VkFence mFence{ VK_NULL_HANDLE };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FENCEIMPL_HPP
