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

    enum class CreateError
    {
        INTERNAL_ERROR = CO_ERROR_INTERNAL,
    };

    virtual ~Fence() = default;

    enum class WaitResult
    {
        SUCCESS        = CO_SUCCESS,
        TIMEOUT        = CO_ERROR_TIMEOUT,
        INTERNAL_ERROR = CO_ERROR_INTERNAL,
    };

    virtual WaitResult wait(uint64_t timeout) = 0;

    virtual void reset() = 0;
};

} // namespace Coral

struct CoFence_T
{
    std::shared_ptr<Coral::Fence> impl;
};

#endif // !CORAL_FENCE_HPP