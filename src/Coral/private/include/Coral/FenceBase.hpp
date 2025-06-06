#ifndef CORAL_FENCEBASE_HPP
#define CORAL_FENCEBASE_HPP

#include <Coral/Fence.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class FenceBase : public Coral::Fence
    {
    public:

        FenceBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_FENCEBASE_HPP
