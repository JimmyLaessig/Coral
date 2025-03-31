#ifndef CORAL_VULKANCOMMANDQUEUEIMPL_HPP
#define CORAL_VULKANCOMMANDQUEUEIMPL_HPP

#include <Coral/CommandQueue.hpp>

#include "Vulkan.hpp"

#include <mutex>
#include <thread>
#include <unordered_map>
#include <future>
#include <unordered_set>


namespace Coral::Vulkan
{

class ContextImpl;

class CommandQueueImpl : public Coral::CommandQueue
{
public:
	CommandQueueImpl() = default;

	CommandQueueImpl(ContextImpl* context, VkQueue queue, uint32_t index, uint32_t mQueueFamilyIndex);

	virtual ~CommandQueueImpl();

	std::expected<Coral::CommandBufferPtr, Coral::CommandBufferCreationError> createCommandBuffer(const Coral::CommandBufferCreateConfig& config) override;

	bool submit(const Coral::CommandBufferSubmitInfo& info, Fence* fence) override;

	bool submit(const Coral::PresentInfo& info) override;

	bool waitIdle() override;

	ContextImpl& context();

	uint32_t getQueueIndex()
	{
		return mQueueIndex;
	}

	VkCommandPool getVkCommandPool();

	VkQueue getVkQueue();

private:

	void awaitStagingBufferReturnTasks();

	Coral::Vulkan::ContextImpl* mContext{ nullptr };

	std::mutex mQueueProtection;

	VkQueue mQueue{ VK_NULL_HANDLE };

	uint32_t mQueueIndex{ 0 };

	uint32_t mQueueFamilyIndex{ 0 };

	std::unordered_map<std::thread::id, VkCommandPool> mCommandPools;

	std::atomic<size_t> mStagingBuffersInFlight{ 0 };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_COMMANDQUEUEIMPL_HPP