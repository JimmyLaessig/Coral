#ifndef CORAL_VULKAN_FENCEIMPL_HPP
#define CORAL_VULKAN_FENCEIMPL_HPP

#include "../FenceBase.hpp"

#include "ContextImpl.hpp"


namespace Coral::Vulkan
{

class FenceImpl: public Coral::FenceBase
{
public:

	using FenceBase::FenceBase;

	virtual ~FenceImpl();

	std::optional<Coral::FenceCreationError> init();

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

	bool wait() override;

	void reset() override;

	VkFence getVkFence();

private:

	VkFence mFence{ VK_NULL_HANDLE };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FENCEIMPL_HPP
