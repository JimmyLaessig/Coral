#ifndef CORAL_IMAGE_HPP
#define CORAL_IMAGE_HPP

#include <Coral/Image.h>

#include <cstdint>

#include <memory>

namespace Coral
{

/*
 * Representation of a multidimensional array of pixel data which can be used for for texturing or render attachments
 */
class CORAL_API Image
{
public:

    using CreateConfig = CoImageCreateConfig;

    /*!
     * Error codes for Image creation
     */ 
    enum class CreateError
    {
        INTERNAL_ERROR
    };

    virtual ~Image() = default;

    /*!
     * \brief Get the width of the Image
     * \return The width of the Image in pixels
     */
    virtual uint32_t width() const = 0;

    /*!
     * \brief Get the height of the Image
     * \return The height of the Image in pixels
     */
    virtual uint32_t height() const = 0;

    /*!
     * \brief Get the format of the Image
     * \return The pixel format of the Image
     */
    virtual CoPixelFormat format() const = 0;

    /*!
     * \brief Get the number of mipmap levels
     * \return The number of mipmap levels in the Image
     */
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
