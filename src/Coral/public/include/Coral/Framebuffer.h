#ifndef CORAL_FRAMEBUFFER_H
#define CORAL_FRAMEBUFFER_H

#include <Coral/Framebuffer.h>

#include <Coral/Image.h>


struct CoFramebuffer_T;

// Opaque handle to a framebuffer object
typedef CoFramebuffer_T* CoFramebuffer;

// Structure specifying the parameters of a color attachment
typedef struct
{
	// Image of the attachment
	/**
	 * Image format must be a color format.
	 */
	CoImage image;

	// Binding index of the color attachment
	uint32_t binding;

} CoColorAttachment;

// Structure specifying the parameters of a newly created framebuffer object
typedef struct  
{
	// Pointer to a list of color attachments
	const CoColorAttachment* pColorAttachments;

	// Number of entries in pColorAttachments
	uint32_t colorAttachmentCount;

	// Optional depth-stencil attachment
	/** 
	 * The format of the image must be a depth-stencil format.
	 */
	CoImage depthAttachment;

} CoFramebufferCreateConfig;


// Structure specifying the attributes of a color attachment
typedef struct
{
	// Format of the attachment
	CoPixelFormat format;

	// Binding index of the attachment
	uint32_t binding;

} CoColorAttachmentInfo;

// Structure specifying the attributes of a depth-stencil attachment
typedef struct
{
	// Format of the attachment
	CoPixelFormat format;

} CoDepthStencilAttachmentInfo;

// Structure specifying the layout of a framebuffer
typedef struct  
{
	// Pointer to a list of CoColorAttachmentInfo structures
	const CoColorAttachmentInfo* pColorAttachments;

	// Number of elements in the pColorAttachments array
	uint32_t colorAttachmentCount;

	// Pointer to a CoDepthStencilAttachmentInfo instance.
	/**
	 * If nullptr, the framebuffer is created without a depth-stencil attachment.
	 */
	const CoDepthStencilAttachmentInfo* depthStencilAttachment;

} CoFramebufferLayout;

/// \brief Create a new framebuffer
/**
 * \param context The context that creates the framebuffer object.
 * \param config Pointer to a CoFramebufferCreateConfig instance containing parameters affecting the framebuffer creation.
 * \param framebuffer[out] Pointer to a CoFramebuffer handle in which the resulting framebuffer object is returned.
 * \return Returns CO_SUCCESS if framebuffer creation was successful. Otherwise one of the
           CO_FRAME_BUFFER_CREATION_ERROR_* values is returned.
 */
CORAL_API CoResult coContextCreateFramebuffer(CoContext context, const CoFramebufferCreateConfig* config, CoFramebuffer* framebuffer);

/// \brief Destroy the framebuffer
/**
 * \param framebuffer The framebuffer to destroy
 */
CORAL_API void coDestroyFramebuffer(CoFramebuffer framebuffer);

/// Get the layout of the framebuffer
/**
 * \param framebuffer The framebuffer. Must be a valid CoFramebuffer handle.
 * \param[out] layout Pointer to a CoFramebufferLayout structure in which the layout is returned.
 */
CORAL_API void coFramebufferGetLayout(const CoFramebuffer framebuffer, CoFramebufferLayout* pLayout);

#endif // !CORAL_FRAMEBUFFER_H
