#include "FenceImpl.hpp"

using namespace Coral::Vulkan;


FenceImpl::~FenceImpl()
{
    if (mFence != VK_NULL_HANDLE)
    {
        vkDestroyFence(context().getVkDevice(), mFence, nullptr);
    }
}


std::optional<Coral::Fence::CreateError>
FenceImpl::init(const Fence::CreateConfig& config)
{
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    
    if (config.createSignaled)
    {
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    if (vkCreateFence(context().getVkDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS)
    {
        return Coral::Fence::CreateError::INTERNAL_ERROR;
    }

    return {};
}


VkFence
FenceImpl::getVkFence()
{
    return mFence;
}


Coral::Fence::WaitResult
FenceImpl::wait(uint64_t timeout)
{
    auto result = vkWaitForFences(context().getVkDevice(), 1, &mFence, VK_TRUE, timeout);
    if (result == VK_SUCCESS)
    {
        return Coral::Fence::WaitResult::SUCCESS;
    }
    else if (result == VK_TIMEOUT)
    {
        return Coral::Fence::WaitResult::TIMEOUT;
    }
    return Coral::Fence::WaitResult::INTERNAL_ERROR;
}


void
FenceImpl::reset()
{
    vkResetFences(context().getVkDevice(), 1, &mFence);
}
