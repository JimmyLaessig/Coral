#ifndef CORAL_SURFACEBASE_HPP
#define CORAL_SURFACEBASE_HPP

#include <Coral/Surface.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class SurfaceBase : public Coral::Surface
    {
    public:

        SurfaceBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_SURFACEBASE_HPP
