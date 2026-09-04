#ifndef CORAL_VULKAN_DESCRIPTORSETPOOL_HPP
#define CORAL_VULKAN_DESCRIPTORSETPOOL_HPP

#include "DescriptorSetImpl.hpp"

#include <unordered_map>
#include <vector>

namespace Coral::Vulkan
{
class ContextImpl;

/*!
 * Structure representing the size of a VkDescriptorSet
 */
struct DescriptorSetSize
{
    uint32_t samplerCount{ 0 };
    uint32_t combinedImageSamplerCount{ 0 };
    uint32_t sampledImageCount{ 0 };
    uint32_t storageImageCount{ 0 };
    uint32_t uniformTexelBufferCount{ 0 };
    uint32_t storageTexelBufferCount{ 0 };
    uint32_t uniformBufferCount{ 0 };
    uint32_t storageBufferCount{ 0 };
    uint32_t uniformBufferDynamicCount{ 0 };
    uint32_t storageBufferDynamicCount{ 0 };
    uint32_t inputAttachmentCount{ 0 };
};

/*!
 * Structure representing the capacity of one VkDescriptorPool
 */
struct PoolCapacity
{
    uint32_t samplerCount{ 0 };
    uint32_t combinedImageSamplerCount{ 0 };
    uint32_t sampledImageCount{ 0 };
    uint32_t storageImageCount{ 0 };
    uint32_t uniformTexelBufferCount{ 0 };
    uint32_t storageTexelBufferCount{ 0 };
    uint32_t uniformBufferCount{ 0 };
    uint32_t storageBufferCount{ 0 };
    uint32_t uniformBufferDynamicCount{ 0 };
    uint32_t storageBufferDynamicCount{ 0 };
    uint32_t inputAttachmentCount{ 0 };
    uint32_t descriptorSetCount{ 0 };
};

/*!
 * Class managing the allocation and deallocation of VkDescriptorSet
 */
class DescriptorSetPool
{
public:

    DescriptorSetPool(ContextImpl& context);

    ~DescriptorSetPool();

    /*!
     * \brief Allocates a descriptor set from the pool.
     * \param config The configuration for creating the descriptor set.
     */
    std::pair<VkDescriptorSetLayout, VkDescriptorSet> allocateDescriptorSet(const DescriptorSet::CreateConfig& config);

    /*!
     * \brief Frees a descriptor set back to the pool.
     * \param descriptorSet The descriptor set to free.
     * \param pool The descriptor pool from which the descriptor set was allocated.
     */
    void freeDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet descriptorSet);

private:

    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout, const DescriptorSetSize& size);

    PoolCapacity calculatePoolCapacity(const DescriptorSetSize& minSize);

    constexpr static PoolCapacity sDefaultPoolCapacity = {
        .samplerCount              = 32,
        .combinedImageSamplerCount = 256,
        .sampledImageCount         = 32,
        .storageImageCount         = 0,
        .uniformTexelBufferCount   = 0,
        .storageTexelBufferCount   = 0,
        .uniformBufferCount        = 256,
        .storageBufferCount        = 256,
        .uniformBufferDynamicCount = 0,
        .storageBufferDynamicCount = 0,
        .inputAttachmentCount      = 0,
        .descriptorSetCount        = 32,
    };

    constexpr static uint32_t sGrowFactor = 2;

    ContextImpl& mContext;

    struct Pool
    {
        //! The Vulkan descriptor pool handle
        VkDescriptorPool pool{ VK_NULL_HANDLE };
        //! The capacity of the pool
        PoolCapacity capacity{};
        //! The number of descriptor sets allocated from this pool
        uint32_t setCount{ 0 };
    };

    std::vector<Pool> mDescriptorPools;
    std::unordered_map<VkDescriptorSet, uint32_t> mLookUpTable;

}; // class DescriptorSetPool

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_DESCRIPTORSETPOOL_HPP
