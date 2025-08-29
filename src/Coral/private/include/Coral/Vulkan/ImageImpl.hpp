#ifndef CORAL_VULKAN_IMAGEIMPL_HPP
#define CORAL_VULKAN_IMAGEIMPL_HPP

#include <Coral/Image.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

namespace Coral::Vulkan
{

class ImageImpl : public Coral::Image
	            , public Resource
	            , std::enable_shared_from_this<ImageImpl>
{
public:

	using Resource::Resource;

	virtual ~ImageImpl();

	bool init(VkImage image, Coral::PixelFormat format, uint32_t width, uint32_t height, uint32_t mipLevelCount, VkImageLayout layout);

	std::optional<Coral::ImageCreationError> init(const Coral::ImageCreateConfig& config);

	VkImageLayout getVkImageLayout() const;

	uint32_t width() const override;

	uint32_t height() const override;

	Coral::PixelFormat format() const override;

	uint32_t getMipLevels() const override;

	bool presentable() const override;

	VkImage getVkImage();

	VkImageView getVkImageView();

private:

	VkImage mImage{ VK_NULL_HANDLE };

	VkImageView mImageView{ VK_NULL_HANDLE };

	VmaAllocation mAllocation{ VK_NULL_HANDLE };

	uint32_t mWidth{ 0 };

	uint32_t mHeight{ 0 };

	Coral::PixelFormat mFormat{ };

	uint32_t mMipLevelCount{ 1 };

	VkImageLayout mLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

	bool mIsOwner{ false };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_IMAGEIMPL_HPP
