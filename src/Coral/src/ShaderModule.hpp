#ifndef CORAL_SHADERMODULE_HPP
#define CORAL_SHADERMODULE_HPP

#include <Coral/ShaderModule.h>

#include <string>
#include <memory>

namespace Coral
{

class CORAL_API ShaderModule
{
public:

    using CreateConfig = CoShaderModuleCreateConfig;

    enum class CreateError
    {
        INTERNAL_ERROR
    };

    virtual ~ShaderModule() = default;

    /// Get the shader stage of the shader module
    virtual CoShaderStage shaderStage() const = 0;

    /// Get the name of the shader module
    virtual const std::string& name() const = 0;

    /// Get the entry point of the shader module
    virtual const std::string& entryPoint() const = 0;

    /// Get the descriptor layout of the shader module
    virtual const CoDescriptorLayout& descriptorLayout() const = 0;

    /// Get the attribute layout of the shader module
    virtual const CoAttributeLayout& attributeLayout() const = 0;

}; // class ShaderModule

} // namespace Coral


struct CoShaderModule_T
{
    std::shared_ptr<Coral::ShaderModule> impl;
};

#endif // !CORAL_SHADERMODULE_HPP
