#ifndef CORAL_FENCE_HPP
#define CORAL_FENCE_HPP

#include <Coral/Fence.h>

#include <memory>

namespace Coral
{

// A fence is a GPU-CPU synchronization mechanic
class CORAL_API Fence
{
public:

    using CreateConfig = CoFenceCreateConfig;

    enum CreateError
    {
        INTERNAL_ERROR = CO_ERROR_INTERNAL,
    };

    virtual ~Fence() = default;

    virtual bool wait() = 0;

    virtual void reset() = 0;
};

} // namespace Coral

struct CoFence_T
{
    std::shared_ptr<Coral::Fence> impl;
};

#endif // !CORAL_FENCE_HPP