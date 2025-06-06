#include <Coral/Vulkan/BufferViewImpl.hpp>


using namespace Coral::Vulkan;


std::optional<Coral::BufferViewCreationError>
BufferViewImpl::init(const Coral::BufferViewCreateConfig& config)
{
	if (!config.buffer)
	{
		return Coral::BufferViewCreationError::INVALID_BUFFER;
	}

	if (config.count == 0)
	{
		return Coral::BufferViewCreationError::EMPTY_VIEW;
	}

	mStride		     = config.stride > 0 ? config.stride : Coral::sizeInBytes(config.attribute);
	mBuffer		     = static_cast<Coral::Vulkan::BufferImpl*>(config.buffer);
	mByteOffset	     = config.offset;
	mAttributeFormat = config.attribute;
	mCount		     = config.count;

	// Calculate the minimum size that the buffer requires for this view to be valid
	auto minBufferByteSize = mByteOffset + (mCount - 1) * mStride + Coral::sizeInBytes(mAttributeFormat);

	if (minBufferByteSize  > config.buffer->size())
	{
		return Coral::BufferViewCreationError::INVALID_SIZE;
	}

	// Views on IndexedBuffers must be tightly packed
	if (mBuffer->type() == BufferType::INDEX_BUFFER && mStride != Coral::sizeInBytes(mAttributeFormat))
	{
		return Coral::BufferViewCreationError::INVALID_STRIDE;
	}

	return {};
}


uint32_t
BufferViewImpl::offset() const
{ 
	return mByteOffset;
}


uint32_t
BufferViewImpl::stride() const
{ 
	return mStride;
}


uint32_t
BufferViewImpl::count() const
{ 
	return mCount;
}


Coral::AttributeFormat
BufferViewImpl::attributeFormat() const
{ 
	return mAttributeFormat;
}


Coral::Buffer*
BufferViewImpl::buffer()
{ 
	return mBuffer;
}


const Coral::Buffer*
BufferViewImpl::buffer() const
{ 
	return mBuffer;
}