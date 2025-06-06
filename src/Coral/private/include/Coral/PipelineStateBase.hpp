#ifndef CORAL_PIPELINESTATEBASE_HPP
#define CORAL_PIPELINESTATEBASE_HPP

#include <Coral/PipelineState.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class PipelineStateBase : public Coral::PipelineState
    {
    public:

        PipelineStateBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_PIPELINESTATEBASE_HPP
