#ifndef CORAL_VULKAN_BUFFERVIEWIMPL_HPP
#define CORAL_VULKAN_BUFFERVIEWIMPL_HPP

#include <Coral/BufferView.hpp>

#include "BufferImpl.hpp"
#include "ContextImpl.hpp"

#include <cstdint>

namespace Coral::Vulkan
{

class BufferViewImpl : public Coral::BufferView
{
public:

	std::optional<Coral::BufferViewCreationError> init(Coral::Vulkan::ContextImpl& context, const Coral::BufferViewCreateConfig& config);

	uint32_t offset() const override;

	uint32_t stride() const override;

	uint32_t count() const override;

	AttributeFormat attributeFormat() const override;

	Coral::Buffer* buffer() override;

	const Coral::Buffer* buffer() const override;

private:

	uint32_t mByteOffset{ 0 };

	uint32_t mStride{ 0 };

	uint32_t mCount{ 0 };

	AttributeFormat mAttributeFormat{ AttributeFormat::VEC3F };

	Coral::Vulkan::BufferImpl* mBuffer{ nullptr };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_BUFFERVIEWIMPL_HPP