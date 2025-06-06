#ifndef CORAL_VULKAN_SHADERMODULEIMPL_HPP
#define CORAL_VULKAN_SHADERMODULEIMPL_HPP

#include <Coral/ShaderModuleBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>

#include <span>
#include <string_view>
#include <vector>

namespace Coral::Vulkan
{

class ShaderModuleImpl : public Coral::ShaderModuleBase
{
public:

	using ShaderModuleBase::ShaderModuleBase;

	virtual ~ShaderModuleImpl();

	std::optional<ShaderModuleCreationError> init(const Coral::ShaderModuleCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

	ShaderStage shaderStage() const override;

	const std::string& name() const override;

	const std::string& entryPoint() const override;

	std::span<const AttributeBindingLayout> inputAttributeBindingLayout() const override;

	std::span<const AttributeBindingLayout> outputAttributeBindingLayout() const override;

	std::span<const DescriptorBindingLayout> descriptorBindingLayout() const override;

	VkShaderModule getVkShaderModule();

private:

	bool reflect(std::span<const std::byte> spirvCode);

	std::string mName;

	std::vector<AttributeBindingLayout> mInputDescriptions;

	std::vector<AttributeBindingLayout> mOutputDescriptions;

	std::vector<DescriptorBindingLayout> mDescriptorBindings;

	std::string mEntryPoint;

	Coral::ShaderStage mShaderStage{ Coral::ShaderStage::VERTEX };

	VkShaderModule mShaderModule{ VK_NULL_HANDLE };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SHADERMODULEIMPL_HPP
