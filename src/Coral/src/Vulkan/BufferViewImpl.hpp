#ifndef CORAL_VULKAN_BUFFERVIEWIMPL_HPP
#define CORAL_VULKAN_BUFFERVIEWIMPL_HPP

#include "../BufferViewBase.hpp"

#include "BufferImpl.hpp"

#include <cstdint>

namespace Coral::Vulkan
{

class BufferViewImpl : public Coral::BufferViewBase
{
public:

	using BufferViewBase::BufferViewBase;

	std::optional<Coral::BufferViewCreationError> init(const Coral::BufferViewCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

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