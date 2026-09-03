
#include "DescriptorPool.hpp"
#include "ContextImpl.hpp"

#include <array>
#include <ranges>

using namespace Coral::Vulkan;

DescriptorPool::DescriptorPool(ContextImpl& context)
    : mContext(context)
{
}


DescriptorPool::~DescriptorPool()
{
    for (auto& pool : mDescriptorPools)
    {
        vkDestroyDescriptorPool(mContext.getVkDevice(), pool.pool, nullptr);
    }
}


VkDescriptorSet
DescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout& layout)
{
    VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &layout;

    for (auto [index, pool]: std::views::enumerate(mDescriptorPools))
    {
        allocInfo.descriptorPool = pool.pool;
        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
        if (vkAllocateDescriptorSets(mContext.getVkDevice(), &allocInfo, &descriptorSet) == VK_SUCCESS)
        {
            pool.setCount++;
            mLookUpTable[descriptorSet] = index;
            return descriptorSet;
        }
    }

    // TODO Adjust descriptor pool sizes to make sure we are able to allocate the descriptor set
    std::array<VkDescriptorPoolSize, 11> poolSizes = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.maxSets       = 1000; // Set a reasonable maximum number of descriptor sets for the new pool
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkDescriptorPool newPool{ VK_NULL_HANDLE };
    if (vkCreateDescriptorPool(mContext.getVkDevice(), &poolInfo, nullptr, &newPool) != VK_SUCCESS)
    {
        // Handle error
        return VK_NULL_HANDLE;
    }

    mDescriptorPools.push_back({ newPool, 0 });

    return allocateDescriptorSet(layout);
}


void
DescriptorPool::freeDescriptorSet(VkDescriptorSet descriptorSet)
{
    auto iter = mLookUpTable.find(descriptorSet);
    if (iter == mLookUpTable.end())
    {
        assert(false);
        return;
    }

    auto index = iter->second;
    auto& pool = mDescriptorPools[iter->second];

    mLookUpTable.erase(descriptorSet);

    vkFreeDescriptorSets(mContext.getVkDevice(), pool.pool, 1, &descriptorSet);

    pool.setCount--;

    // Remove the pool if it has no more allocated descriptor sets
    if (pool.setCount == 0)
    {
        vkDestroyDescriptorPool(mContext.getVkDevice(), pool.pool, nullptr);
        mDescriptorPools.erase(mDescriptorPools.begin() + index);
        // Update the lookup table for all descriptor sets that were allocated from pools after the
        // removed pool
        for (auto& [_, idx] : mLookUpTable)
        {
            if (idx > index)
            {
                idx--;
            }
        }
    }
}
