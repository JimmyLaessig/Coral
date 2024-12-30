#include "SemaphoreImpl.hpp"

using namespace Coral::Vulkan;

std::optional<Coral::SemaphoreCreationError>
SemaphoreImpl::init(ContextImpl& context)
{
	mContext = &context;
	VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	timelineCreateInfo.pNext			= NULL;
	timelineCreateInfo.semaphoreType	= VK_SEMAPHORE_TYPE_TIMELINE;
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


bool
SemaphoreImpl::wait(uint64_t timeout)
{
	constexpr static uint64_t ONE = 1;
	VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
	waitInfo.semaphoreCount = 1;
	waitInfo.pValues		= &ONE;
	waitInfo.pSemaphores	= &mSemaphore;
	return vkWaitSemaphores(mContext->getVkDevice(), &waitInfo, timeout) == VK_SUCCESS;
}