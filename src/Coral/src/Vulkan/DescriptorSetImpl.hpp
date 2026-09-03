#ifndef CORAL_VULKAN_DESCRIPTORSETIMPL_HPP
#define CORAL_VULKAN_DESCRIPTORSETIMPL_HPP

#include "DescriptorSet.hpp"
#include "Fwd.hpp"
#include "Resource.hpp"
#include "Vulkan.hpp"

#include <expected>
#include <vector>

namespace Coral::Vulkan
{

class DescriptorSetImpl : public Coral::DescriptorSet, 
                          public Resource,
                          public std::enable_shared_from_this<DescriptorSetImpl>
{
public:

    using Resource::Resource;

    ~DescriptorSetImpl();

    std::optional<Coral::DescriptorSet::CreateError> init(const DescriptorSet::CreateConfig& config);

    VkDescriptorSet getVkDescriptorSet() const { return mDescriptorSet; }

private:

    VkDescriptorSetLayout mLayout{ VK_NULL_HANDLE };
    VkDescriptorSet mDescriptorSet{ VK_NULL_HANDLE };

}; // class DescriptorSetIImpl

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_DESCRIPTORSETIMPL_HPP
