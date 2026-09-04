#include "DescriptorSetPool.hpp"
#include "ContextImpl.hpp"

#include <ranges>
#include <span>

using namespace Coral::Vulkan;

DescriptorSetPool::DescriptorSetPool(ContextImpl& context)
    : mContext(context)
{
}


DescriptorSetPool::~DescriptorSetPool()
{
    for (auto& pool : mDescriptorPools)
    {
        vkDestroyDescriptorPool(mContext.getVkDevice(), pool.pool, nullptr);
    }
}


PoolCapacity
DescriptorSetPool::calculatePoolCapacity(const DescriptorSetSize& minSize)
{
    PoolCapacity capacity = sDefaultPoolCapacity;

    // Exponential growth of the pool capacity based on the last pool's capacity or the default
    // capacity if no pools exist
    if (!mDescriptorPools.empty())
    { 
        capacity = mDescriptorPools.back().capacity;

        capacity.samplerCount              *= sGrowFactor;
        capacity.combinedImageSamplerCount *= sGrowFactor;
        capacity.sampledImageCount         *= sGrowFactor; 
        capacity.storageImageCount         *= sGrowFactor; 
        capacity.uniformTexelBufferCount   *= sGrowFactor; 
        capacity.storageTexelBufferCount   *= sGrowFactor; 
        capacity.uniformBufferCount        *= sGrowFactor; 
        capacity.storageBufferCount        *= sGrowFactor; 
        capacity.uniformBufferDynamicCount *= sGrowFactor; 
        capacity.storageBufferDynamicCount *= sGrowFactor; 
        capacity.inputAttachmentCount      *= sGrowFactor;
        capacity.descriptorSetCount        *= sGrowFactor;
    }

    // Ensure that the new capacity is at least as large as the minimum requested size
    capacity.samplerCount              = std::max(capacity.samplerCount,              minSize.samplerCount),
    capacity.combinedImageSamplerCount = std::max(capacity.combinedImageSamplerCount, minSize.combinedImageSamplerCount),
    capacity.sampledImageCount         = std::max(capacity.sampledImageCount,         minSize.sampledImageCount),
    capacity.storageImageCount         = std::max(capacity.storageImageCount,         minSize.storageImageCount),
    capacity.uniformTexelBufferCount   = std::max(capacity.uniformTexelBufferCount,   minSize.uniformTexelBufferCount),
    capacity.storageTexelBufferCount   = std::max(capacity.storageTexelBufferCount,   minSize.storageTexelBufferCount),
    capacity.uniformBufferCount        = std::max(capacity.uniformBufferCount,        minSize.uniformBufferCount),
    capacity.storageBufferCount        = std::max(capacity.storageBufferCount,        minSize.storageBufferCount),
    capacity.uniformBufferDynamicCount = std::max(capacity.uniformBufferDynamicCount, minSize.uniformBufferDynamicCount),
    capacity.storageBufferDynamicCount = std::max(capacity.storageBufferDynamicCount, minSize.storageBufferDynamicCount),
    capacity.inputAttachmentCount      = std::max(capacity.inputAttachmentCount,      minSize.inputAttachmentCount);

    return capacity;
}

VkDescriptorType convert(CoDescriptorType type)
{
    switch (type)
    {
    case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case CO_DESCRIPTOR_TYPE_IMAGE:                  return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case CO_DESCRIPTOR_TYPE_SAMPLER:                return VK_DESCRIPTOR_TYPE_SAMPLER;
    case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    std::unreachable();
}


std::pair<VkDescriptorSetLayout, VkDescriptorSet>
DescriptorSetPool::allocateDescriptorSet(const DescriptorSet::CreateConfig& config)
{
    auto device = mContext.getVkDevice();

    auto inputBindings = std::span(config.pDescriptorBindings, config.descriptorBindingCount);

    DescriptorSetSize sizes{};

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& binding : inputBindings)
    {
        auto& vkBinding = bindings.emplace_back();
        vkBinding.binding            = binding.binding;
        vkBinding.descriptorCount    = 1;
        vkBinding.stageFlags         = VK_SHADER_STAGE_ALL;
        vkBinding.pImmutableSamplers = VK_NULL_HANDLE;
 
        switch (binding.type)
        {
            case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                sizes.uniformBufferCount++;
                break;
            case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                sizes.storageBufferCount++;
                break;
            case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                sizes.combinedImageSamplerCount++;
                break;
            case CO_DESCRIPTOR_TYPE_IMAGE:
                vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                sizes.sampledImageCount++;
                break;
            case CO_DESCRIPTOR_TYPE_SAMPLER:
                vkBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                sizes.samplerCount++;
                break;
        };
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    layoutInfo.pBindings    = bindings.data();
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());

    VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
    {
        return { VK_NULL_HANDLE, VK_NULL_HANDLE };
    }

    auto set = allocateDescriptorSet(layout, sizes);
    if (set == VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
        layout = VK_NULL_HANDLE;
    }
    return std::pair(layout, set);
}


VkDescriptorSet
DescriptorSetPool::allocateDescriptorSet(VkDescriptorSetLayout layout, const DescriptorSetSize& size)
{
    VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &layout;

    for (auto [index, pool] : std::views::enumerate(mDescriptorPools))
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

    // Calculate the required capacity for the new pool based on the requested size, set count
    // and factor in empirical default values for each descriptor type
    auto capacity = calculatePoolCapacity(size);

    std::vector<VkDescriptorPoolSize> vkPoolSizes = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,                capacity.samplerCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, capacity.combinedImageSamplerCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          capacity.sampledImageCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          capacity.storageImageCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   capacity.uniformTexelBufferCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   capacity.storageTexelBufferCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         capacity.uniformBufferCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         capacity.storageBufferCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, capacity.uniformBufferDynamicCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, capacity.storageBufferDynamicCount },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       capacity.inputAttachmentCount }
    };

    std::erase_if(vkPoolSizes, [](const VkDescriptorPoolSize& poolSize) { return poolSize.descriptorCount == 0; });

    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.maxSets       = capacity.descriptorSetCount;
    poolInfo.pPoolSizes    = vkPoolSizes.data();
    poolInfo.poolSizeCount = static_cast<uint32_t>(vkPoolSizes.size());
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkDescriptorPool newPool{ VK_NULL_HANDLE };
    if (vkCreateDescriptorPool(mContext.getVkDevice(), &poolInfo, nullptr, &newPool) != VK_SUCCESS)
    {
        // Handle error
        return VK_NULL_HANDLE;
    }

    mDescriptorPools.push_back({ newPool, capacity, 0 });

    return allocateDescriptorSet(layout, size);
}


void
DescriptorSetPool::freeDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet descriptorSet)
{
    auto iter = mLookUpTable.find(descriptorSet);
    if (iter == mLookUpTable.end())
    {
        assert(false);
        return;
    }
    auto device = mContext.getVkDevice();

    vkDestroyDescriptorSetLayout(device, layout, nullptr);

    auto index = iter->second;
    auto& pool = mDescriptorPools[iter->second];
    
    mLookUpTable.erase(descriptorSet);

    vkFreeDescriptorSets(device, pool.pool, 1, &descriptorSet);

    pool.setCount--;

    // Remove the pool if it has no more allocated descriptor sets
    if (pool.setCount == 0)
    {
        vkDestroyDescriptorPool(device, pool.pool, nullptr);
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
