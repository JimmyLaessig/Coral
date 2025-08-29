#include <Coral/Vulkan/FenceImpl.hpp>

using namespace Coral::Vulkan;


FenceImpl::~FenceImpl()
{
	if (mFence != VK_NULL_HANDLE)
	{
		vkDestroyFence(context().getVkDevice(), mFence, nullptr);
	}
}


std::optional<Coral::FenceCreationError>
FenceImpl::init()
{
	VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

	if (vkCreateFence(context().getVkDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS)
	{
		return Coral::FenceCreationError::INTERNAL_ERROR;
	}

	return {};
}


VkFence
FenceImpl::getVkFence()
{
	return mFence;
}


bool
FenceImpl::wait()
{
	return vkWaitForFences(context().getVkDevice(), 1, &mFence, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
}


void
FenceImpl::reset()
{
	vkResetFences(context().getVkDevice(), 1, &mFence);
}
