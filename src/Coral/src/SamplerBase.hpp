#ifndef CORAL_SAMPLERBASE_HPP
#define CORAL_SAMPLERBASE_HPP

#include <Coral/Sampler.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class SamplerBase : public Coral::Sampler
    {
    public:

        SamplerBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_SAMPLERBASE_HPP
