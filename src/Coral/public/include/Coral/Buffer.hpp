#ifndef CORAL_BUFFER_HPP
#define CORAL_BUFFER_HPP

#include <Coral/System.hpp>
#include <Coral/Types.hpp>

#include <cstddef>
#include <memory>

namespace Coral
{

/// The type of the buffer
enum class BufferType
{
	VERTEX_BUFFER,
	INDEX_BUFFER,
	UNIFORM_BUFFER,
	STORAGE_BUFFER
};


/// Configuration to create a Buffer
struct BufferCreateConfig
{
	/// The size of the buffer in bytes
	size_t size{ 0 };

	/// The type of the buffer
	BufferType type{ BufferType::VERTEX_BUFFER };

	/// Flag indicating if the buffer's memory should can be mapped to CPU memory
	bool cpuVisible{ false };
};


/// Error codes for Buffer creation
enum class BufferCreationError
{
	/// The requested buffer size is zero.
	INVALID_SIZE,
	/// Creation failed due to an internal error.
	INTERNAL_ERROR,
	/// THe requested buffer size is too large
	OUT_OF_MEMORY,
};


class CORAL_API Buffer
{
public:

	virtual ~Buffer() = default;

	/// Get the size of the buffer in bytes
	virtual size_t size() const = 0;

	/// Get the type of the buffer
	virtual BufferType type() const = 0;

	/// Map the buffer memory to CPU-accessible memory.
	/** 
	 * This function only returns a valid pointer if the buffer was created with the `cpuVisible` option enabled. 
	 * The memory CPU <-> GPU memory synchronization functions `map` and `unmap` should not be called whilst the buffer
	 * is in use by a command buffer.
	 */
	virtual std::byte* map() = 0;

	/// Unmap the buffer memory, mapped previously by `map()`.
	/*
	 * Updates to the buffer's CPU data are only synchronized with the buffer's GPU data once `unmap` is called.
	 */
	virtual bool unmap() = 0;

};


/// Helper function to control duration of mapped memory using RAII
class CORAL_API MemoryMapHandle
{
public:

	/// Create a new MemoryMapHandle object
	MemoryMapHandle(Buffer& buffer)
		: mBuffer(buffer)
		, mData(buffer.map())
	{}

	~MemoryMapHandle()
	{
		if (mData)
		{
			mBuffer.unmap();
		}
	}

	std::byte* data() { return mData; }

private:
	
	Buffer& mBuffer;

	std::byte* mData{ nullptr };
};

} // namespace Coral

#endif // !CORAL_BUFFER_HPP