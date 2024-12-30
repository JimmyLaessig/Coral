#ifndef CORAL_IMAGE_HPP
#define CORAL_IMAGE_HPP

#include <Coral/System.hpp>
#include <Coral/Types.hpp>

#include <cstddef>
#include <cstdint>


namespace Coral
{

/// Configuration to create an Image
struct ImageCreateConfig
{
	/// The width of the Image
	uint32_t width{ 0 };

	/// The height of the Image
	uint32_t height{ 0 };

	/// Flag indicating if the Image should be equipped with multiple levels of detail for minified sampling of the Image.
	bool hasMips{ false };

	/// The format and type of the texel blocks contained in the Image
	PixelFormat format{ PixelFormat::RGBA8_UI };
};

/// Error codes for Image creation
enum class ImageCreationError
{
	INTERNAL_ERROR
};

///
class CORAL_API Image
{
public:

	virtual ~Image() = default;

	/// The width of the Image
	virtual uint32_t width() const = 0;

	/// The height of the Image
	virtual uint32_t height() const = 0;

	/// Get the pixel format of the Image
	virtual PixelFormat format() const = 0;

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

#endif // !CORAL_IMAGE_HPP
