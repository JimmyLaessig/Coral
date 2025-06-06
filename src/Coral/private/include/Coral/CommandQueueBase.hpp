#ifndef CORAL_COMMANDQUEUEBASE_HPP
#define CORAL_COMMANDQUEUEBASE_HPP

#include <Coral/CommandQueue.hpp>

#include "ContextBase.hpp"

namespace Coral
{

class CommandBufferBase;

class CommandQueueBase : public Coral::CommandQueue
{
public:

    CommandQueueBase(Coral::ContextBase& context)
        : mContext(context)
    {
    }

    ContextBase& context() { return mContext; }

    virtual void destroyCommandBuffer(CommandBufferBase* commandBuffer) = 0;

private:

    ContextBase& mContext;

};

} // namespace Coral

#endif // !CORAL_COMMANDQUEUEBASE_HPP
