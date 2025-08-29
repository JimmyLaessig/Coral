#ifndef CORAL_VULKAN_FENCEIMPL_HPP
#define CORAL_VULKAN_FENCEIMPL_HPP

#include <Coral/Fence.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

namespace Coral::Vulkan
{

class FenceImpl : public Coral::Fence
	            , public Resource
	            , std::enable_shared_from_this<FenceImpl>
{
public:

	using Resource::Resource;

	virtual ~FenceImpl();

	std::optional<Coral::FenceCreationError> init();

	bool wait() override;

	void reset() override;

	VkFence getVkFence();

private:

	VkFence mFence{ VK_NULL_HANDLE };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FENCEIMPL_HPP
