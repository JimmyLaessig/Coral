#ifndef CORAL_SHADERMODULEBASE_HPP
#define CORAL_SHADERMODULEBASE_HPP

#include <Coral/ShaderModule.hpp>

#include "ContextBase.hpp"

namespace Coral
{

    class ShaderModuleBase : public Coral::ShaderModule
    {
    public:

        ShaderModuleBase(Coral::ContextBase& context)
            : mContext(context)
        {
        }

        ContextBase& context() { return mContext; }

    private:

        ContextBase& mContext;

    };

} // namespace Coral

#endif // !CORAL_SHADERMODULEBASE_HPP
