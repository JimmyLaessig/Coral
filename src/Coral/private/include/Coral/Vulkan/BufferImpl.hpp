#ifndef CORAL_VULKAN_BUFFERIMPL_HPP
#define CORAL_VULKAN_BUFFERIMPL_HPP

#include <Coral/BufferBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>

namespace Coral::Vulkan
{

class BufferImpl : public Coral::BufferBase
{
public:

	using BufferBase::BufferBase;

	virtual ~BufferImpl();

	std::optional<Coral::BufferCreationError> init(const BufferCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

	size_t size() const override;

	BufferType type() const override;

	std::byte* map() override;

	bool unmap() override;

	VkBuffer getVkBuffer();

private:

	VkBuffer mBuffer{ VK_NULL_HANDLE };

	VmaAllocation mAllocation{ VK_NULL_HANDLE };

	BufferType mType{ BufferType::STORAGE_BUFFER };

	size_t mSize{ 0 };

	bool mCpuVisible{ false };

	std::byte* mMapped{ nullptr };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_BUFFERIMPL_HPP