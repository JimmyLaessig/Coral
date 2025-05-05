#include "BufferImpl.hpp"

#include "VulkanFormat.hpp"

#include <cassert>
#include <string>
#include <span>

using namespace Coral::Vulkan;


BufferImpl::~BufferImpl()
{
	if (mBuffer != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(contextImpl().getVmaAllocator(), mBuffer, mAllocation);
	}
}


std::optional<Coral::BufferCreationError>
BufferImpl::init(const Coral::BufferCreateConfig& config)
{
	if (config.size == 0)
	{
		return Coral::BufferCreationError::INVALID_SIZE;
	}

	mType		= config.type;
	mSize		= config.size;
	mCpuVisible = config.cpuVisible;

	uint32_t queueFamilyIndex = contextImpl().getQueueFamilyIndex();

	VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.pQueueFamilyIndices   = &queueFamilyIndex;
	createInfo.queueFamilyIndexCount = 1;
	createInfo.sharingMode			 = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.size					 = mSize;
	createInfo.usage				 = convert(mType);

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	if (mCpuVisible)
	{
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	}

	VmaAllocationInfo allocInfo{};
	switch (vmaCreateBuffer(contextImpl().getVmaAllocator(), &createInfo, &allocCreateInfo, &mBuffer, &mAllocation, &allocInfo))
	{
		case VK_SUCCESS:
			return {};
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return Coral::BufferCreationError::OUT_OF_MEMORY;
		default:
			return Coral::BufferCreationError::INTERNAL_ERROR;
	}
}


VkBuffer
BufferImpl::getVkBuffer()
{
	return mBuffer;
}


size_t BufferImpl::size() const
{
	return mSize;
}

Coral::BufferType
BufferImpl::type() const
{
	return mType;
}


std::byte*
BufferImpl::map()
{
	if (!mCpuVisible)
	{
		return nullptr;
	}

	// Already mapped
	if (mMapped)
	{
		return nullptr;
	}

	void* data{ nullptr };

	if (vmaMapMemory(contextImpl().getVmaAllocator(), mAllocation, &data) != VK_SUCCESS)
	{
		return nullptr;
	}

	mMapped = (std::byte*)data;
	return mMapped;
}


bool
BufferImpl::unmap()
{
	if (!mMapped)
	{
		return false;
	}

	mMapped = nullptr;
	vmaUnmapMemory(contextImpl().getVmaAllocator(), mAllocation);

	return true;
}
