#ifndef CORAL_VULKAN_IMAGEIMPL_HPP
#define CORAL_VULKAN_IMAGEIMPL_HPP

#include <Coral/Image.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

namespace Coral::Vulkan
{
/// Representation of a multidimensional array of pixel data which can be used for for texturing or render attachments
/**
 * Vulkan memory model allows for very fine-grained performance-oriented design for image memory layout. Ideally, for
 * the most suitable memory layout is chosen for each task. In pratice, this micro optimization is a complex task and
 * keeping track of the memory layout can be a heavy burden on the programmer and is often causes of rendering issues
 * or validation errors. For those practical reasons, Coral abstracts memory layout transitions away and uses a reduced
 * set of layouts. 
 * New images are created with VK_IMAGE_LAYOUT_UNDEFINED. Based on a usage hint, Coral derives the preferred image
 * layout in an attempt to reduce unneeded image layout transitions. Images that are prematurely used as shader inputs
 * the preferred image layout is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, whereas for images that are mostly used as 
 * attachments the preferred image layout is VK_IMAGE_LAYOUT_SHADER_ATTACHMENT_OPTIMAL.
 *
 * The ImageImpl class attempts to keep track of the current image layout in an attempt to reduce programmer errors
 * when dealing with layout transitions manually. As a result, the developer must be cautious and try to avoid mixing 
 * updates, descriptor usage or render attachment in order to prevent data races. Sequential use should be okay. Since
 * layout transitions happen during command buffer execution, recorded transitions commands should always be executed,
 * otherwise the CPU representation is out of sync with the GPU representation of the current memory layout.
 */
class ImageImpl : public Coral::Image
	            , public Resource
	            , std::enable_shared_from_this<ImageImpl>
{
public:

	using Resource::Resource;

	virtual ~ImageImpl();

	bool init(VkImage image, Coral::PixelFormat format, uint32_t width, uint32_t height, uint32_t mipLevelCount, ImageUsageHint usageHint);

	std::optional<Coral::ImageCreationError> init(const Coral::ImageCreateConfig& config);

	uint32_t width() const override;

	uint32_t height() const override;

	Coral::PixelFormat format() const override;

	uint32_t getMipLevels() const override;

	VkImage getVkImage();

	VkImageView getVkImageView();

	bool presentable() const override;

	VkImageLayout getPreferredImageLayout();

	static void cmdTransitionImageLayout(VkCommandBuffer buffer, ImageImpl& image, VkImageLayout layout, uint32_t mipLevel, uint32_t levelCount, 
		                                 VkAccessFlagBits srcAccessMask,
		                                 VkAccessFlags dstAccessMask,
		                                 VkPipelineStageFlags srcStageFlags,
		                                 VkPipelineStageFlags dstStageFlags);

private:

	VkImage mImage{ VK_NULL_HANDLE };

	VkImageView mImageView{ VK_NULL_HANDLE };

	VmaAllocation mAllocation{ VK_NULL_HANDLE };

	uint32_t mWidth{ 0 };

	uint32_t mHeight{ 0 };

	Coral::PixelFormat mFormat{ };

	uint32_t mMipLevelCount{ 1 };

	VkImageLayout mPreferredImageLayout;
	
	std::vector<VkImageLayout> mCurrentLayout;

	bool mIsOwner{ false };
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_IMAGEIMPL_HPP
