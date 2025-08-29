#ifndef CORAL_COMMANDQUEUE_HPP
#define CORAL_COMMANDQUEUE_HPP

#include <Coral/System.hpp>
#include <Coral/CoralFwd.hpp>
#include <Coral/CommandBuffer.hpp>

#include <expected>
#include <span>

namespace Coral
{

struct CommandBufferSubmitInfo
{
	/// The command buffers to to execute in batch. 
	/**
	 * The order of command buffers in the list dictates the order of
	 * submission and beginning of execution, but are allowed to proceed
	 * independently after that and complete out of order.
	 * 
	 * \note: The caller must ensure that the waitSemaphores are submitted as signalSemaphores of a command buffer
	 * submission. Otherwise, execution will not start.
	 */
	std::span<CommandBuffer*> commandBuffers;

	/// List of semaphores to wait for before execution of the command buffer can start.
	std::span<Semaphore*> waitSemaphores;

	/// List of semaphores to signal once execution of the command buffer has finished.
	std::span<Semaphore*> signalSemaphores;
};


struct PresentInfo
{
	/// List of semaphores to wait for before presentation can start.
	/**
	 * \note: The caller must ensure that the waitSemaphores are submitted as signalSemaphores of a command buffer 
	 * submission. Otherwise, execution will not start.
	 */
	std::span<Semaphore*> waitSemaphores;

	/// The swapchain to present
	Swapchain* swapchain{ nullptr };
};


/// A Command Queue executes submitted work
class CORAL_API CommandQueue
{
public:

	virtual ~CommandQueue() = default;

    virtual std::expected<Coral::CommandBufferPtr, Coral::CommandBufferCreationError> createCommandBuffer(const Coral::CommandBufferCreateConfig& config) = 0;

	/// Submit a sequence of command buffers to a queue
	virtual bool submit(const CommandBufferSubmitInfo& info, Fence* fence) = 0;

	/// Wait until the queue is idle.
	virtual bool waitIdle() = 0;

	/// Present the last rendered image to the screen.
	virtual bool submit(const PresentInfo& info) = 0;
};

} // namespace Coral

#endif // !CORAL_COMMANDQUEUE_HPP
