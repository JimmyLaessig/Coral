#include "SemaphoreImpl.hpp"

using namespace Coral::Vulkan;


SemaphoreImpl::~SemaphoreImpl()
{
	if (mSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(contextImpl().getVkDevice(), mSemaphore, nullptr);
	}
}


std::optional<Coral::SemaphoreCreationError>
SemaphoreImpl::init()
{
	VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	timelineCreateInfo.pNext			= NULL;
	timelineCreateInfo.semaphoreType	= VK_SEMAPHORE_TYPE_BINARY;
	timelineCreateInfo.initialValue		= 0;

	VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	info.pNext = &timelineCreateInfo;

	if (vkCreateSemaphore(contextImpl().getVkDevice(), &info, nullptr, &mSemaphore) != VK_SUCCESS)
	{
		return Coral::SemaphoreCreationError::INTERNAL_ERROR;
	}

	return {};
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