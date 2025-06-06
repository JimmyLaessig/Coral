#ifndef CORAL_BUFFERVIEW_HPP
#define CORAL_BUFFERVIEW_HPP

#include <Coral/System.hpp>
#include <Coral/Types.hpp>
#include <Coral/Buffer.hpp>

#include <cstdint>

namespace Coral
{

/// Configuration to create a BufferView
struct BufferViewCreateConfig
{
	/// The buffer upon which to create a view
	Buffer* buffer{ nullptr };

	/// The number of elements in the BufferView
	uint32_t count{ 0 };

	/// The format of the elements in the BufferView
	AttributeFormat attribute{ AttributeFormat::VEC4F };

	/// The offset from the base address of the buffer to the first element of the BufferView
	uint32_t offset{ 0 };

	/// The byte stride between elements in the BufferView
	/**
	 * If zero, the elements are assumed to be tightly packed and the stride will be set to the byte size of one
	 * element.
	 * 
	 *\Note: BufferViews on IndexBuffers must be tightly packed. Hence, stride must be zero or the byte size of one
	 * element. 
	 */
	uint32_t stride{ 0 };
};

/// Error codes for BufferView creation
enum class BufferViewCreationError
{
	/// Creation failed because the provided Buffer was nullptr.
	INVALID_BUFFER,
	/// Creation failed because the view would access elements beyond the allocated memory of the underlying Buffer.
	INVALID_SIZE,
	/// Creation failed because the view was requested to be empty.
	EMPTY_VIEW,
	/// Creation failed because the underlying Buffer requires tightly-packed BufferViews.
	INVALID_STRIDE,
};

/// A BufferView provides the GPU structured access to a Buffer's data in the shader
class CORAL_API BufferView
{
public:

	virtual ~BufferView() = default;

	/// Get the offset from the base address of the Buffer
	virtual uint32_t offset() const = 0;

	/// Get the stride
	virtual uint32_t stride() const = 0;

	/// Get the number of elements contained in the view
	virtual uint32_t count() const = 0;

	/// Get the format of the attribute
	virtual AttributeFormat attributeFormat() const = 0;

	/// Get the Buffer underneath the view
	virtual const Buffer* buffer() const = 0;

	/// Get the Buffer underneath the view
	virtual Buffer* buffer() = 0;

	/// Get the render context
	virtual class Context& context() = 0;
};

} // namespace Coral

#endif // !CORAL_BUFFERVIEW_HPP