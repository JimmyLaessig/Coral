#ifndef CORAL_FRAMEBUFFERBASE_HPP
#define CORAL_FRAMEBUFFERBASE_HPP

#include <Coral/Framebuffer.hpp>

#include "ContextBase.hpp"

namespace Coral
{

class FramebufferBase : public Coral::Framebuffer
{
public:

    FramebufferBase(Coral::ContextBase& context)
        : mContext(context)
    {
    }

    ContextBase& context() { return mContext; }

private:

    ContextBase& mContext;

};

} // namespace Coral

#endif // !CORAL_FRAMEBUFFERBASE_HPP
