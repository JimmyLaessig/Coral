#include "SemaphoreImpl.hpp"

using namespace Coral::Vulkan;

std::optional<Coral::SemaphoreCreationError>
SemaphoreImpl::init(ContextImpl& context)
{
	mContext = &context;
	VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	timelineCreateInfo.pNext			= NULL;
	timelineCreateInfo.semaphoreType	= VK_SEMAPHORE_TYPE_BINARY;
	timelineCreateInfo.initialValue		= 0;

	VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	info.pNext = &timelineCreateInfo;

	if (vkCreateSemaphore(mContext->getVkDevice(), &info, nullptr, &mSemaphore) != VK_SUCCESS)
	{
		return Coral::SemaphoreCreationError::INTERNAL_ERROR;
	}

	return {};
}


SemaphoreImpl::~SemaphoreImpl()
{
	if (mSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(mContext->getVkDevice(), mSemaphore, nullptr);
	}
}


VkSemaphore
SemaphoreImpl::getVkSemaphore()
{
	return mSemaphore;
}


const VkSemaphore 
SemaphoreImpl::getVkSemaphore() const
{
	return mSemaphore;
}