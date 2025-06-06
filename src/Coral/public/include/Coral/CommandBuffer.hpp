#ifndef CORAL_COMMANDBUFFER_HPP
#define CORAL_COMMANDBUFFER_HPP

#include <Coral/Buffer.hpp>
#include <Coral/BufferView.hpp>
#include <Coral/Image.hpp>
#include <Coral/Sampler.hpp>
#include <Coral/PipelineState.hpp>

#include <functional>
#include <optional>
#include <span>
#include <string_view>

namespace Coral
{

/// Configuration to create a CommandBuffer
struct CommandBufferCreateConfig
{
	/// The name of the CommandBuffer
	std::string_view name;
};

/// Error codes for CommandBuffer creation
enum class CommandBufferCreationError
{
	/// CommandBuffer creation failed due to an internal error.
	INTERNAL_ERROR,
};

//enum class PrimitiveType
//{
//	Triangles, 
//	Lines, 
//	Points,
//};
//
//enum class BlitAttachment
//{
//	COLOR,
//	DEPTH,
//	COLOR_AND_DEPTH
//};
//
//struct BlitCommandConfig
//{
//	Framebuffer* source;
//	Framebuffer* target;
//
//	Rectangle sourceViewport{};
//	Rectangle targetViewport{};
//
//	bool blitColorAttachment{ true };
//	bool blitDepthAttachment{ false };
//};
//

///
struct ClearColor
{
	float color[4];
	uint32_t attachment;
};

///
struct ClearDepth
{
	float depth{ 1.f };
	uint32_t stencil{ 0 };
};

struct BeginRenderPassInfo
{
	Framebuffer* framebuffer{ nullptr };

	std::span<ClearColor> clearColor;
	std::optional<ClearDepth> clearDepth;
};


struct EndRenderPassInfo
{
	Framebuffer* framebuffer{ nullptr };
};


struct CopyBufferInfo
{
	/// Source buffer to copy data from
	Buffer* source{ nullptr };

	/// Destination buffer to copy data to
	Buffer* dest{ nullptr };

	/// Starting offset from the start of the source buffer (in bytes)
	size_t sourceOffset{ 0 };

	/// Starting offset from the start of the destination buffer (in bytes)
	size_t destOffset{ 0 };

	/// Number of bytes to copy
	size_t size{ 0 };
};

struct CopyImageInfo
{
	Buffer* source{ nullptr };

	Image* dest{ nullptr };
};


struct DrawIndexInfo
{
	uint32_t indexCount{ 0 };

	uint32_t firstIndex{ 0 };
};

enum class ViewportMode
{
	Y_UP,
	Y_DOWN
};

struct ViewportInfo
{
	Rectangle viewport{ 0, 0, 0, 0 };
	float minDepth{ 0.f };
	float maxDepth{ 0.f };
	ViewportMode mode{ ViewportMode::Y_UP };
};


struct UpdateBufferDataInfo
{
	/// The buffer to update
	Coral::Buffer* buffer{ nullptr };

	/// Offset to the begin of the buffer
	uint32_t offset{ 0 };

	/// The data to be put into the buffer
	std::span<const std::byte> data;
};


struct UpdateImageDataInfo
{
	/// The image to update
	Coral::Image* image{ nullptr };
	
	/// The data to be put into the image
	std::span<const std::byte> data;

	/// Flag indicating if the image data of the mip levels should be updated
	bool updateMips{ false };
};


class CORAL_API CommandBuffer
{
public:

	virtual ~CommandBuffer() = default;

	/// Begin command recording
	virtual bool begin() = 0;

	/// End command recording
	/**
	 * Command buffers can only be scheduled for execution the command buffer
	 * is no longer in recording state
	 */
	virtual bool end() = 0;

	/// Clear the color attachments of the bound framebuffer
	virtual bool cmdClearImage(Coral::Image* image, const ClearColor& clearColor) = 0;

	virtual bool cmdBeginRenderPass(const BeginRenderPassInfo& info) = 0;

	virtual bool cmdEndRenderPass() = 0;

	virtual bool cmdUpdateBufferData(const UpdateBufferDataInfo& info) = 0;

	virtual bool cmdUpdateImageData(const UpdateImageDataInfo& info) = 0;

	virtual bool cmdCopyBuffer(const CopyBufferInfo& info) = 0;

	virtual bool cmdCopyImage(const CopyImageInfo& info) = 0;

	virtual bool cmdBindVertexBuffer(Coral::BufferView* vertexBuffer, uint32_t binding) = 0;

	virtual bool cmdBindIndexBuffer(Coral::BufferView* indexBuffer) = 0;

	virtual bool cmdBindPipeline(Coral::PipelineState* pipeline) = 0;

	virtual bool cmdDrawIndexed(const DrawIndexInfo& info) = 0;

	virtual bool cmdSetViewport(const Coral::ViewportInfo& info) = 0;

	virtual void cmdBindDescriptor(Coral::Buffer* buffer, uint32_t binding) = 0;

	virtual void cmdBindDescriptor(Coral::Image* image, Coral::Sampler* sampler, uint32_t binding) = 0;

	virtual void cmdBindDescriptor(Coral::Sampler* sampler, uint32_t binding) = 0;

	virtual void cmdBindDescriptor(Coral::Image* image, uint32_t binding) = 0;

	virtual void cmdBlitImage(Coral::Image* source, Coral::Image* dest) = 0;
};



} // namespace Coral

#endif // !CORAL_COMMANDBUFFER_HPP