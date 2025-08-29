#include <Coral/Vulkan/CommandQueueImpl.hpp>

#include <Coral/Vulkan/CommandBufferImpl.hpp>
#include <Coral/Vulkan/FenceImpl.hpp>
#include <Coral/Vulkan/SemaphoreImpl.hpp>
#include <Coral/Vulkan/SwapchainImpl.hpp>

#include <mutex>
#include <vector>
#include <future>
#include <ranges>


using namespace Coral::Vulkan;

CommandQueueImpl::CommandQueueImpl(ContextImpl& context, VkQueue queue, uint32_t queueIndex, uint32_t queueFamilyIndex)
	: Resource(context)
	, mQueue(queue)
	, mQueueIndex(queueIndex)
	, mQueueFamilyIndex(queueFamilyIndex)
{
}


CommandQueueImpl::~CommandQueueImpl()
{
	waitIdle();
	
	for (auto [_, commandPool] : mCommandPools)
	{
		vkDestroyCommandPool(context().getVkDevice(), commandPool, nullptr);
	}
}


std::expected<Coral::CommandBufferPtr, Coral::CommandBufferCreationError>
CommandQueueImpl::createCommandBuffer(const Coral::CommandBufferCreateConfig& config)
{
	auto cb = new Coral::Vulkan::CommandBufferImpl(*this);

	if (!cb->init(config))
	{
		delete cb;
		return std::unexpected(Coral::CommandBufferCreationError::INTERNAL_ERROR);
	}

	return Coral::CommandBufferPtr(cb);
}


bool
CommandQueueImpl::submit(const Coral::CommandBufferSubmitInfo& info, Coral::Fence* fence)
{
	std::lock_guard lock(mQueueProtection);

	std::vector<VkSemaphore> waitSemaphores;
	std::vector<VkPipelineStageFlags> waitFlags;
	for (auto semaphore : info.waitSemaphores)
	{
		waitSemaphores.push_back(static_cast<const Vulkan::SemaphoreImpl*>(semaphore)->getVkSemaphore());
		// Wait with execution of all commands until all semaphores are signaled
		waitFlags.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	}

	std::vector<VkSemaphore> signalSemaphores;
	for (auto semaphore : info.signalSemaphores)
	{
		signalSemaphores.push_back(static_cast<Vulkan::SemaphoreImpl*>(semaphore)->getVkSemaphore());
	}

	// Collect all command buffers and staging buffers used in the command buffers
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<std::shared_ptr<Coral::Buffer>> stagingBuffers;

	for (auto commandBuffer : info.commandBuffers)
	{
		auto commandBufferImpl = static_cast<Vulkan::CommandBufferImpl*>(commandBuffer);
		commandBuffers.push_back(commandBufferImpl->getVkCommandBuffer());

		auto commandBufferStagingBuffers = commandBufferImpl->getStagingBuffers();

		stagingBuffers.insert(stagingBuffers.end(), 
							  std::move_iterator(commandBufferStagingBuffers.begin()), 
							  std::move_iterator(commandBufferStagingBuffers.end()));
	}

	// Coral does automatically create staging buffers for CPU <-> GPU copy operations. To reduce buffer allocations, 
	// Coral uses a context-wide buffer pool to reuse pre-existing staging buffers. Hence, the staging buffers must be
	// kept in memory until the command buffer execution is finished. For that, we spawn an async task that waits for 
	// the semaphore to signal and releases the buffers afterwards. To prevent race condition on destruction the
	// command queue keeps track of the count of in-flight staging buffers. Only after the semaphore is signaled
	// decrement the count. Idling the command queue must wait until the in-flight staging buffer count is 0.

	// If an external fence is used, reuse this fence, otherwise create a temporary fence object.
	VkFence vkFence = fence ? static_cast<Coral::Vulkan::FenceImpl*>(fence)->getVkFence() : VK_NULL_HANDLE;
	bool ownsFence = false;
	if (!stagingBuffers.empty() && !vkFence)
	{
		VkFenceCreateInfo info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		if (vkCreateFence(context().getVkDevice(), &info, nullptr, &vkFence) != VK_SUCCESS)
		{
			// TODO: FATAL ERROR
			return false;
		}
		ownsFence = true;
	}

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pNext				= nullptr;
	submitInfo.pCommandBuffers		= commandBuffers.data();
	submitInfo.commandBufferCount	= static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pSignalSemaphores	= signalSemaphores.data();
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pWaitSemaphores		= waitSemaphores.data();
	submitInfo.waitSemaphoreCount	= static_cast<uint32_t>(waitSemaphores.size());
	submitInfo.pWaitDstStageMask	= waitFlags.data();

	if (vkQueueSubmit(mQueue, 1, &submitInfo, vkFence) != VK_SUCCESS)
	{
		return false;
	}

	// Add the async task that waits for the command buffer execution to release the staging buffers
	if (!stagingBuffers.empty())
	{
		// Increment the count of in-flight staging buffers
		mStagingBuffersInFlight += stagingBuffers.size();

		// Transfer ownership of the staging buffers to the async task
		auto task = [stagingBuffers = std::move(stagingBuffers), fence = vkFence, ownsFence = ownsFence, this]() mutable
		{
			// Wait until the fence is signaled
			vkWaitForFences(context().getVkDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

			auto count = stagingBuffers.size();
			// Clear all staging buffers (this reduces the use count of the shared ptr which effectively  returns
			// them to the pool.
			stagingBuffers.clear();
			// Decrement the count of in-flight staging buffers
			mStagingBuffersInFlight -= count;

			if (ownsFence)
			{
				vkDestroyFence(context().getVkDevice(), fence, nullptr);
			}
		};

		auto future = std::async(std::launch::async, std::move(task));
	}

	return true;
}


void
CommandQueueImpl::awaitStagingBufferReturnTasks()
{
	while (mStagingBuffersInFlight > 0);
}


bool
CommandQueueImpl::waitIdle()
{
	if (mQueue == VK_NULL_HANDLE)
	{
		return false;
	}

	// Wait for the queue to finish processing all commands
	auto success = vkQueueWaitIdle(mQueue) == VK_SUCCESS;

	awaitStagingBufferReturnTasks();

	return success;
}


bool
CommandQueueImpl::submit(const Coral::PresentInfo& info)
{
	auto swapchain = static_cast<Coral::Vulkan::SwapchainImpl*>(info.swapchain);
	swapchain->present(*this, info.waitSemaphores);
	return true;
}


VkCommandPool
CommandQueueImpl::getVkCommandPool()
{
	std::lock_guard lock(mQueueProtection);

	auto id = std::this_thread::get_id();

	auto iter = mCommandPools.find(id);
	if (iter != mCommandPools.end())
	{
		return iter->second;
	}

	VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.queueFamilyIndex = mQueueFamilyIndex;
	createInfo.flags			= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool commandPool{ VK_NULL_HANDLE };
	if (vkCreateCommandPool(context().getVkDevice(), &createInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	mCommandPools[id] = commandPool;

	return commandPool;
}


VkQueue
CommandQueueImpl::getVkQueue()
{
	return mQueue;
}
