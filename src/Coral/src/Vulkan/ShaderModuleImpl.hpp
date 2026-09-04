#ifndef CORAL_VULKAN_SHADERMODULEIMPL_HPP
#define CORAL_VULKAN_SHADERMODULEIMPL_HPP

#include "ShaderModule.hpp"
#include "Resource.hpp"

#include <span>
#include <optional>

namespace Coral::Vulkan
{

/*!
 * Implementation of the ShaderModule interface using the Vulkan backend
 */
class ShaderModuleImpl : public Coral::ShaderModule
                       , public Resource
                       , public std::enable_shared_from_this<ShaderModuleImpl>
{
public:

    using Resource::Resource;

    virtual ~ShaderModuleImpl();

    std::optional<ShaderModule::CreateError> init(const Coral::ShaderModule::CreateConfig& config);

    ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

    CoShaderStage shaderStage() const override;

    const std::string& name() const override;

    const std::string& entryPoint() const override;

    const CoDescriptorLayout& descriptorLayout() const override;

    const CoAttributeLayout& attributeLayout() const override;

    VkShaderModule getVkShaderModule();

private:

    bool reflect(std::span<const uint32_t> spirvCode);

    std::string mName;

    std::string mEntryPoint;

    CoShaderStage mShaderStage{ CO_SHADER_STAGE_VERTEX };

    CoDescriptorLayout mDescriptorLayout;

    CoAttributeLayout mAttributeLayout;

    VkShaderModule mShaderModule{ VK_NULL_HANDLE };

}; // class ShaderModuleImpl

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SHADERMODULEIMPL_HPP
