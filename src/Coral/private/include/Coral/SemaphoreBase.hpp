#ifndef CORAL_SEMAPHOREBASE_HPP
#define CORAL_SEMAPHOREBASE_HPP

#include <Coral/Semaphore.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class SemaphoreBase : public Coral::Semaphore
    {
    public:

        SemaphoreBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_SEMAPHOREBASE_HPP
