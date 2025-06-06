#ifndef CORAL_COMMANDBUFFERBASE_HPP
#define CORAL_COMMANDBUFFERBASE_HPP

#include <Coral/CommandBuffer.hpp>

#include "CommandQueueBase.hpp"

namespace Coral
{

    class CommandBufferBase : public Coral::CommandBuffer
    {
    public:

        CommandBufferBase(Coral::CommandQueueBase& commandQueue)
            : mCommandQueue(commandQueue)
        {
        }

        CommandQueueBase& commandQueue() { return mCommandQueue; }

        ContextBase& context() { return mCommandQueue.context(); }

    private:

        CommandQueueBase& mCommandQueue;

    };

} // namespace Coral

#endif // !CORAL_COMMANDBUFFERBASE_HPP
