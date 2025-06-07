#ifndef CORAL_SWAPCHAINBASE_HPP
#define CORAL_SWAPCHAINBASE_HPP

#include <Coral/Swapchain.hpp>

#include <Coral/ContextBase.hpp>

namespace Coral
{

    class SwapchainBase : public Coral::Swapchain
    {
    public:

        SwapchainBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_SWAPCHAINBASE_HPP
