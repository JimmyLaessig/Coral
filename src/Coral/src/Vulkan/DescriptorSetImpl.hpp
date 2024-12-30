#ifndef CORAL_VULKAN_DESCRIPTORSETIMPL_HPP
#define CORAL_VULKAN_DESCRIPTORSETIMPL_HPP

#include <Coral/DescriptorSet.hpp>

#include "ContextImpl.hpp"

namespace Coral::Vulkan
{

class DescriptorSetImpl: public Coral::DescriptorSet
{
public:

	virtual ~DescriptorSetImpl();

	std::optional<Coral::DescriptorSetCreationError> init(Coral::Vulkan::ContextImpl& context, const Coral::DescriptorSetCreateConfig& config);

	VkDescriptorSet getVkDescriptorSet();

private:

	ContextImpl* mContext{ nullptr };

	VkDescriptorSetLayout mDescriptorSetLayout{ VK_NULL_HANDLE };

	VkDescriptorSet mDescriptorSet{ VK_NULL_HANDLE };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_DESCRIPTORSETIMPL_HPP
