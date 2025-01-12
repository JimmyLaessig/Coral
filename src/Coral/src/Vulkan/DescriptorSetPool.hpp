#ifndef CORAL_VULKAN_DESCRIPTORSETPOOL_HPP
#define CORAL_VULKAN_DESCRIPTORSETPOOL_HPP

#include "ContextImpl.hpp"
#include "DescriptorSetImpl.hpp"

#include <array>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace Coral::Vulkan
{


struct DescriptorPoolDefaultSizes
{
	float avgSamplerCountPerDescriptorSet{ 3.f };
	float avgImageCountPerDescriptorSet{ 3.f };
	float avgCombinedImageSamplerCountPerDescriptorSet{ 3.f };
	float avgUniformBufferCountPerDescriptorSet{ 2.f };
	float avgStorageBufferCountPerDescriptorSet{ 1.f };
};


struct DescriptorPoolData
{
	VkDevice mDevice{ VK_NULL_HANDLE };
	/// The underlying Vulkan Descriptor Pool
	VkDescriptorPool mPool{ VK_NULL_HANDLE };
	/// The theoretical number of descriptor sets that can be allocated from the pool
	uint32_t mCapacity{ 0 };
	/// The actual number of descriptor sets allocated from this pool
	uint32_t mSize{ 0 };
	/// The number of failed allocations in the descriptor pool (due to the pool not having enough space left
	uint32_t mFailedAllocations{ 0 };

	~DescriptorPoolData();

	DescriptorPoolData() = default;
	DescriptorPoolData(const DescriptorPoolData&) = delete;
	DescriptorPoolData(DescriptorPoolData&&) = delete;
	DescriptorPoolData& operator=(const DescriptorPoolData&) = delete;
	DescriptorPoolData& operator=(DescriptorPoolData&&) = delete;
};


class ContextImpl;

class DescriptorSetPool
{
public:

	DescriptorSetPool(Coral::Vulkan::ContextImpl& context);

	~DescriptorSetPool();

	VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout);

	void freeDescriptorSet(VkDescriptorSet descriptorSet);

	void shrinkToFit();

private:

	ContextImpl* mContext{ nullptr };

	DescriptorPoolDefaultSizes mDefaultPoolSizes{};

	uint32_t mDefaultPoolCapacity{ 1000 };

	std::vector<std::unique_ptr<DescriptorPoolData>> mPools;

	std::unordered_map<VkDescriptorSet, DescriptorPoolData*> mDescriptorSetLookUp;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_DESCRIPTORSETPOOL_HPP
