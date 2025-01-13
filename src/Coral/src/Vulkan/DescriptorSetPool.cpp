#include "DescriptorSetPool.hpp"

#include "../Visitor.hpp"
#include "BufferImpl.hpp"
#include "ContextImpl.hpp"
#include "ImageImpl.hpp"
#include "SamplerImpl.hpp"

#include <array>
#include <ranges>

using namespace Coral::Vulkan;

namespace
{

std::unique_ptr<DescriptorPoolData>
createDescriptorPool(VkDevice device, const DescriptorPoolDefaultSizes& defaultSizes, uint32_t capacity)
{
	auto result = std::make_unique<DescriptorPoolData>();

	std::array poolSizes =
	{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,				 static_cast<uint32_t>(capacity * defaultSizes.avgSamplerCountPerDescriptorSet) },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,			 static_cast<uint32_t>(capacity * defaultSizes.avgImageCountPerDescriptorSet) },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(capacity * defaultSizes.avgCombinedImageSamplerCountPerDescriptorSet) },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		 static_cast<uint32_t>(capacity * defaultSizes.avgUniformBufferCountPerDescriptorSet) },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,		 static_cast<uint32_t>(capacity * defaultSizes.avgStorageBufferCountPerDescriptorSet) },
	};
	
	for (auto& poolSize: poolSizes)
	{
		poolSize.descriptorCount = std::max(1u, poolSize.descriptorCount);
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes	   = poolSizes.data();
	descriptorPoolCreateInfo.maxSets	   = capacity;
	descriptorPoolCreateInfo.flags	       = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	VkDescriptorPool pool{ VK_NULL_HANDLE };
	if (vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &pool) != VK_SUCCESS)
	{
		return nullptr;
	}
	
	result->mDevice			   = device;
	result->mPool			   = pool;
	result->mFailedAllocations = 0;
	result->mCapacity		   = capacity;
	result->mSize			   = 0;

	return result;
}

} // namespace


DescriptorPoolData::~DescriptorPoolData()
{
	if (mPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(mDevice, mPool, nullptr);
	}
}


DescriptorSetPool::DescriptorSetPool(Coral::Vulkan::ContextImpl& context)
	: mContext(&context)
{
}


VkDescriptorSet
DescriptorSetPool::allocateDescriptorSet(VkDescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts		= &layout;

	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

	uint32_t maxSpaceAllocationFailed = 0;

	// Try to allocate the descriptor set
	for (auto& pool : std::ranges::reverse_view(mPools))
	{
		allocateInfo.descriptorPool = pool->mPool;

		auto result = vkAllocateDescriptorSets(mContext->getVkDevice(), &allocateInfo, &descriptorSet);

		if (result == VK_SUCCESS)
		{
			pool->mSize++;
			mDescriptorSetLookUp[descriptorSet] = pool.get();
			return descriptorSet;
		}

		else if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
		{
			maxSpaceAllocationFailed = std::max(pool->mCapacity - pool->mSize, maxSpaceAllocationFailed);
		}
		else
		{
			assert(false);
			return VK_NULL_HANDLE;
		}
	}

	// Allocation failed for all existing DescriptorPools. Create a new descriptor pool and retry

	// Allocation failed for a pool that exceeded the default pool capacity, meaning that the DescriptorSet requires
	// more descriptors than an empty pool can provide. Therefore, we need to increase the default capacity otherwise
	// the newly created pool is also not able to allocate the DescriptorSet. This is a very edge-case scenario and
	// only occurs for very large DescriptorSets and very small DescriptorPools. Nonetheless, we need to deal with this
	// case otherwise we would end up in an endless loop.
	if (maxSpaceAllocationFailed >= mDefaultPoolCapacity)
	{
		mDefaultPoolCapacity *= 2;
	}

	// Create the new pool with the updated capacity
	auto pool = createDescriptorPool(mContext->getVkDevice(), mDefaultPoolSizes, mDefaultPoolCapacity);

	// If we can't allocate anymore pools we are in big trouble
	if (!pool)
	{
		assert(false);
		return VK_NULL_HANDLE;
	}

	// push the new pool to the back of the vector. The loop above iterates over the pools in reverse order. Thus, the
	// new mpool has the highest priority
	mPools.push_back(std::move(pool));

	return allocateDescriptorSet(layout);
}


void
DescriptorSetPool::freeDescriptorSet(VkDescriptorSet descriptorSet)
{
	auto pool = mDescriptorSetLookUp.find(descriptorSet);

	if (pool != mDescriptorSetLookUp.end())
	{
		vkFreeDescriptorSets(mContext->getVkDevice(), pool->second->mPool, 1, &descriptorSet);
		pool->second->mSize--;
		mDescriptorSetLookUp.erase(pool);
	}
	else
	{
		assert(false);
	}
}


DescriptorSetPool::~DescriptorSetPool()
{
}


void
DescriptorSetPool::shrinkToFit()
{

}
