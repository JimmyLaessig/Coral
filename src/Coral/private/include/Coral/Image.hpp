#ifndef CORAL_IMAGE_HPP
#define CORAL_IMAGE_HPP

#include <Coral/Image.h>

#include <cstddef>
#include <cstdint>

#include <memory>


namespace Coral
{

///
class CORAL_API Image
{
public:

	using CreateConfig = CoImageCreateConfig;

	/// Error codes for Image creation
	enum class CreateError
	{
		INTERNAL_ERROR
	};

	virtual ~Image() = default;

	/// The width of the Image
	virtual uint32_t width() const = 0;

	/// The height of the Image
	virtual uint32_t height() const = 0;

	/// Get the pixel format of the Image
	virtual CoPixelFormat format() const = 0;

	/// Get the number of mipmap levels
	virtual uint32_t getMipLevels() const = 0;

	/// Flag indicating if the Image is presentable, e.g. it is part of a swapchain.
	/**
	 * Presentable Image have limitations on usage. They cannot be used for:
	 * * Clearing via cmdClearImage.
	 * * 
	 */
	virtual bool presentable() const = 0;
};

} // namespace Coral

struct CoImage_T
{
	std::shared_ptr<Coral::Image> impl;
};

#endif // !CORAL_IMAGE_HPP
