#ifndef CORAL_VULKAN_BUFFERIMPL_HPP
#define CORAL_VULKAN_BUFFERIMPL_HPP

#include <Coral/Buffer.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

namespace Coral::Vulkan
{

class BufferImpl : public Coral::Buffer,
	               public std::enable_shared_from_this<BufferImpl>,
	               public Resource
{
public:

	using Resource::Resource;

	virtual ~BufferImpl();

	std::optional<Coral::Buffer::CreateError> init(const Buffer::CreateConfig& config);

	size_t size() const override;

	CoBufferType type() const override;

	std::byte* map() override;

	bool unmap() override;

	VkBuffer getVkBuffer();

private:

	VkBuffer mBuffer{ VK_NULL_HANDLE };

	VmaAllocation mAllocation{ VK_NULL_HANDLE };

	CoBufferType mType{ CO_BUFFER_TYPE_STORAGE };

	size_t mSize{ 0 };

	bool mCpuVisible{ false };

	std::byte* mMapped{ nullptr };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_BUFFERIMPL_HPP