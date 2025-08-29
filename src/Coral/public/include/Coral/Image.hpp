#ifndef CORAL_IMAGE_HPP
#define CORAL_IMAGE_HPP

#include <Coral/System.hpp>
#include <Coral/Types.hpp>

#include <cstddef>
#include <cstdint>


namespace Coral
{

enum class ImageUsageHint
{
	// The image is primarily used as frame buffer attachment
	FRAMEBUFFER_ATTACHMENT,
	// The image is primarily used as read-only resource in a shader
	SHADER_READ_ONLY
};


/// Configuration to create an image
struct ImageCreateConfig
{
	/// The width of the image
	uint32_t width{ 0 };

	/// The height of the image
	uint32_t height{ 0 };

	/// Flag indicating if the image should be equipped with multiple levels of detail for minified sampling of the image.
	bool hasMipMaps{ false };

	/// The format and type of the texel blocks contained in the Image
	PixelFormat format{ PixelFormat::RGBA8_UI };

	/// Hint about the usage of the image
	/**
	 * The usage hint helps Coral to optimize memory layout of the image for best performance. Note that the hint is
	 * not binding and images with usage hint SHADER_READ_ONLY can still be used as framebuffer attachments and vice
	 * versa.
	 */
	ImageUsageHint usageHint{ ImageUsageHint::SHADER_READ_ONLY };
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
