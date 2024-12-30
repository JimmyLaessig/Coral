#include "FenceImpl.hpp"

using namespace Coral::Vulkan;

FenceImpl::~FenceImpl()
{
	if (mContext && mFence != VK_NULL_HANDLE)
	{
		vkDestroyFence(mContext->getVkDevice(), mFence, nullptr);
	}
}


std::optional<Coral::FenceCreationError>
FenceImpl::init(ContextImpl& context)
{
	mContext = &context;
	VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

	if (vkCreateFence(mContext->getVkDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS)
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
	return vkWaitForFences(mContext->getVkDevice(), 1, &mFence, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
}


void
FenceImpl::reset()
{
	vkResetFences(mContext->getVkDevice(), 1, &mFence);
}
