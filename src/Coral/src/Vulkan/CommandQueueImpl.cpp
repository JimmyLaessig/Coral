#include "CommandQueueImpl.hpp"

#include "CommandBufferImpl.hpp"
#include "FenceImpl.hpp"
#include "SemaphoreImpl.hpp"
#include "SurfaceImpl.hpp"

#include <mutex>
#include <vector>
#include <future>

using namespace Coral::Vulkan;

CommandQueueImpl::CommandQueueImpl(ContextImpl* context, VkQueue queue, uint32_t queueIndex, uint32_t queueFamilyIndex)
	: mContext(context)
	, mQueue(queue)
	, mQueueIndex(queueIndex)
	, mQueueFamilyIndex(queueFamilyIndex)
{
}


CommandQueueImpl::~CommandQueueImpl()
{
	waitIdle();
	
	if (!mContext)
	{
		return;
	}

	for (auto [_, commandPool] : mCommandPools)
	{
		vkDestroyCommandPool(mContext->getVkDevice(), commandPool, nullptr);
	}
}


std::expected<Coral::CommandBufferPtr, Coral::CommandBufferCreationError>
CommandQueueImpl::createCommandBuffer(const Coral::CommandBufferCreateConfig& config)
{
	auto cb = new Coral::Vulkan::CommandBufferImpl;

	if (!cb->init(*this, config))
	{
		delete cb;
		return std::unexpected(Coral::CommandBufferCreationError::INTERNAL_ERROR);
	}

	return Coral::CommandBufferPtr(cb);
}


bool
CommandQueueImpl::submit(const Coral::CommandBufferSubmitInfo& info, Coral::Fence* fence)
{
	cleanFinishedStagingBufferReturnTasks();

	std::lock_guard lock(mQueueProtection);

	std::vector<VkSemaphore> waitSemaphores;
	std::vector<VkPipelineStageFlags> waitFlags;
	std::vector<uint64_t> waitValues;
	for (auto semaphore : info.waitSemaphores)
	{
		waitSemaphores.push_back(static_cast<Vulkan::SemaphoreImpl*>(semaphore)->getVkSemaphore());
		// Wait with execution of all commands until all semaphores are signaled
		waitFlags.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		waitValues.push_back(1);
	}

	std::vector<VkSemaphore> signalSemaphores;
	std::vector<uint64_t> signalValues;
	for (auto semaphore : info.signalSemaphores)
	{
		signalSemaphores.push_back(static_cast<Vulkan::SemaphoreImpl*>(semaphore)->getVkSemaphore());
		signalValues.push_back(1);
	}

	// Collect all command buffers and staging buffers used in the command buffers
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<Coral::BufferPtr> stagingBuffers;

	for (auto commandBuffer : info.commandBuffers)
	{
		auto commandBufferImpl = static_cast<Vulkan::CommandBufferImpl*>(commandBuffer);
		commandBuffers.push_back(commandBufferImpl->getVkCommandBuffer());

		auto commandBufferStagingBuffers = commandBufferImpl->getStagingBuffers();

		stagingBuffers.insert(stagingBuffers.end(), 
							  std::move_iterator(commandBufferStagingBuffers.begin()), 
							  std::move_iterator(commandBufferStagingBuffers.end()));
	}

	Coral::SemaphorePtr stagingBufferSemaphore;
	if (!stagingBuffers.empty())
	{
		stagingBufferSemaphore = mContext->createSemaphore().value();

		signalSemaphores.push_back(static_cast<Coral::Vulkan::SemaphoreImpl*>(stagingBufferSemaphore.get())->getVkSemaphore());
		signalValues.push_back(1);
	}

	VkTimelineSemaphoreSubmitInfo timelineInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
	timelineInfo.pNext						= nullptr;
	timelineInfo.waitSemaphoreValueCount	= static_cast<uint32_t>(waitValues.size());
	timelineInfo.pWaitSemaphoreValues		= waitValues.data();
	timelineInfo.signalSemaphoreValueCount	= static_cast<uint32_t>(signalValues.size());
	timelineInfo.pSignalSemaphoreValues		= signalValues.data();

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pNext				= &timelineInfo;
	submitInfo.pCommandBuffers		= commandBuffers.data();
	submitInfo.commandBufferCount	= static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pSignalSemaphores	= signalSemaphores.data();
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pWaitSemaphores		= waitSemaphores.data();
	submitInfo.waitSemaphoreCount	= static_cast<uint32_t>(waitSemaphores.size());
	submitInfo.pWaitDstStageMask	= waitFlags.data();

	VkFence vkFence = fence ? static_cast<Coral::Vulkan::FenceImpl*>(fence)->getVkFence() : VK_NULL_HANDLE;

	if (vkQueueSubmit(mQueue, 1, &submitInfo, vkFence) != VK_SUCCESS)
	{
		return false;
	}

	// Coral does automatically create staging buffers for CPU <-> GPU copy operations. To reduce buffer allocations, 
	// Coral uses a context-wide buffer pool to reuse pre-existing staging buffers. However, they must be manually
	// returned once execution of the command buffer has finished. Therefore, we need to spawn an asynchronuous task
	// that waits on the semaphore to signal so that this task can return the borrowed staging buffers to the pool.
	// 
	// To prevent race conditions on destruction, the scheduled tasks are collected and when the device is waiting for
	// idle all scheduled tasks are awaited as well so that the all staging buffers are returned.
	if (stagingBufferSemaphore)
	{
		submitStagingBufferReturnTask(std::move(stagingBufferSemaphore), std::move(stagingBuffers));
	}

	return true;
}


void
CommandQueueImpl::submitStagingBufferReturnTask(Coral::SemaphorePtr&& semaphore, std::vector<Coral::BufferPtr>&& stagingBuffers)
{
	std::lock_guard lock(mReturnTasksInFlightProtection);

	// Create an asynchronuous task that waits for the semaphore and returns the staging buffers to the buffer pool.
	mReturnTasksInFlight.push_back(std::async(std::launch::async, [
			semaphore = Coral::SemaphorePtr(std::move(semaphore)),
			stagingBuffers = std::vector<Coral::BufferPtr>(std::move(stagingBuffers)),
			this]() mutable
		{
			// Wait until the semaphore is signaled
			semaphore->wait();
			// Return the staging buffers
			mContext->returnStagingBuffers(std::move(stagingBuffers));
		}));
}


void
CommandQueueImpl::cleanFinishedStagingBufferReturnTasks()
{
	// Remove all finished return tasks from the list
	std::lock_guard lock(mReturnTasksInFlightProtection);
	std::erase_if(mReturnTasksInFlight, [](auto& future)
	{
		return future.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready;
	});
}


void
CommandQueueImpl::awaitStagingBufferReturnTasks()
{
	// Wait for all return tasks to finish
	std::lock_guard lock(mReturnTasksInFlightProtection);
	for (auto& future : mReturnTasksInFlight)
	{
		future.wait();
	}
	mReturnTasksInFlight.clear();
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


ContextImpl&
CommandQueueImpl::context()
{
	return *mContext;
}


bool
CommandQueueImpl::submit(const Coral::PresentInfo& info)
{
	cleanFinishedStagingBufferReturnTasks();

	std::vector<VkSemaphore> waitSemaphores;
	for (auto semaphore : info.waitSemaphores)
	{
		waitSemaphores.push_back(static_cast<Vulkan::SemaphoreImpl*>(semaphore)->getVkSemaphore());
	}

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pWaitSemaphores	= waitSemaphores.data();

	uint32_t swapchainImageIndex = info.surface->getCurrentSwapchainImageIndex();
	presentInfo.pImageIndices    = &swapchainImageIndex;

	VkSwapchainKHR swapchain   = static_cast<Coral::Vulkan::SurfaceImpl*>(info.surface)->getVkSwapchain();
	presentInfo.pSwapchains    = &swapchain;
	presentInfo.swapchainCount = 1;
	
	return vkQueuePresentKHR(mQueue, &presentInfo);
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
	if (vkCreateCommandPool(mContext->getVkDevice(), &createInfo, nullptr, &commandPool) != VK_SUCCESS)
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
