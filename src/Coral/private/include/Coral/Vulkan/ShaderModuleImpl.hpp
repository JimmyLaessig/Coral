#ifndef CORAL_VULKAN_SHADERMODULEIMPL_HPP
#define CORAL_VULKAN_SHADERMODULEIMPL_HPP

#include <Coral/ShaderModule.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

#include <span>
#include <string_view>
#include <vector>

namespace Coral::Vulkan
{

class ShaderModuleImpl : public Coral::ShaderModule
	                   , public Resource
	                   , public std::enable_shared_from_this<ShaderModuleImpl>
{
public:

	using Resource::Resource;

	virtual ~ShaderModuleImpl();

	std::optional<ShaderModuleCreationError> init(const Coral::ShaderModuleCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

	ShaderStage shaderStage() const override;

	const std::string& name() const override;

	const std::string& entryPoint() const override;

	const AttributeLayout& inputAttributeLayout() const override;

	const AttributeLayout& outputAttributeLayout() const override;

	const DescriptorLayout& descriptorLayout() const override;

	VkShaderModule getVkShaderModule();

private:

	bool reflect(std::span<const std::byte> spirvCode);

	std::string mName;

	AttributeLayout mInputAttributeLayout;

	AttributeLayout mOutputAttributeLayout;

	DescriptorLayout mDescriptorLayout;

	std::string mEntryPoint;

	Coral::ShaderStage mShaderStage{ Coral::ShaderStage::VERTEX };

	VkShaderModule mShaderModule{ VK_NULL_HANDLE };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SHADERMODULEIMPL_HPP
