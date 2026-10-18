#ifndef CORAL_SHADERMODULE_HPP
#define CORAL_SHADERMODULE_HPP

#include <Coral/ShaderModule.h>

#include "ShaderModuleReflection.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>

namespace Coral
{

/*!
 * \brief A shader module maintains the source code of a shader and its reflection data. 
 */
class CORAL_API ShaderModule
{
public:

    struct CreateConfig
    {
        CreateConfig(const CoShaderModuleCreateConfig& config);

        /*!
         * The name of the shader module
         */
        std::string_view name;

        /*!
         * The stage of the shader module
         */
        CoShaderStage stage;

        /*!
         * The source code of the shader module
         */
        std::span<const std::byte> source;

        /*!
         * Name of the entry point
         */
        std::string_view entryPoint;
    };

    enum class CreateError
    {
        INTERNAL_ERROR
    };

    virtual ~ShaderModule() = default;

    /*!
     * \brief Get the shader stage of the shader module
     */
    virtual CoShaderStage shaderStage() const = 0;

    /*!
     * Get the name of the shader module
     */
    virtual const std::string& name() const = 0;

    /*!
     * Get the entry point of the shader module
     */
    virtual const std::string& entryPoint() const = 0;

    /*!
     * \brief Get the reflection information of the shader module
     */
    virtual const ShaderModuleReflectionInfo& getReflectionInfo() const = 0;

}; // class ShaderModule

} // namespace Coral


struct CoShaderModule_T
{
    ~CoShaderModule_T();

    std::shared_ptr<Coral::ShaderModule> impl;

    std::optional<CoShaderModuleReflectionInfo> mReflection;
};

#endif // !CORAL_SHADERMODULE_HPP
