#ifndef CORAL_VULKAN_BUFFERIMPL_HPP
#define CORAL_VULKAN_BUFFERIMPL_HPP

#include <Coral/Buffer.hpp>

#include "ContextImpl.hpp"

namespace Coral::Vulkan
{

class BufferImpl : public Coral::Buffer
{
public:

	virtual ~BufferImpl();

	std::optional<Coral::BufferCreationError> init(ContextImpl& context, const BufferCreateConfig& config);

	VkBuffer getVkBuffer();

	size_t size() const override;

	BufferType type() const override;

	std::byte* map() override;

	bool unmap() override;

private:

	ContextImpl* mContext{ nullptr };

	VkBuffer mBuffer{ VK_NULL_HANDLE };

	VmaAllocation mAllocation{ VK_NULL_HANDLE };

	BufferType mType{ BufferType::STORAGE_BUFFER };

	size_t mSize{ 0 };

	bool mCpuVisible{ false };

	std::byte* mMapped{ nullptr };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_BUFFERIMPL_HPP