#ifndef CORAL_VULKAN_DESCRIPTORPOOL_HPP
#define CORAL_VULKAN_DESCRIPTORPOOL_HPP

#include "Vulkan.hpp"

#include <unordered_map>
#include <set>

namespace Coral::Vulkan
{
class ContextImpl;

class DescriptorPool
{
public:

    DescriptorPool(ContextImpl& context);

    ~DescriptorPool();

    /*!
     * \brief Allocates a descriptor set from the pool.
     * \param layout The descriptor set layout to allocate.
     */
    VkDescriptorSet allocateDescriptorSet(const VkDescriptorSetLayout& layout);

    /*!
     * \brief Frees a descriptor set back to the pool.
     * \param descriptorSet The descriptor set to free.
     * \param pool The descriptor pool from which the descriptor set was allocated.
     */
    void freeDescriptorSet(VkDescriptorSet descriptorSet);

private:

    ContextImpl& mContext;

    float mGrowFactor{ 2.f };

    struct Pool
    {
        //! The Vulkan descriptor pool handle
        VkDescriptorPool pool{ VK_NULL_HANDLE };
        //! The number of descriptor sets allocated from this pool
        uint32_t setCount{ 0 };
    };

    std::vector<Pool> mDescriptorPools;
    std::unordered_map<VkDescriptorSet, uint32_t> mLookUpTable;

}; // class DescriptorPool

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_DESCRIPTORPOOL_HPP
