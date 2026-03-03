#ifndef CORAL_SAMPLER_H
#define CORAL_SAMPLER_H

#include <Coral/System.hpp>
#include <Coral/Context.h>

/// 
typedef enum
{
	/// Return the texture element that is nearest to the specified texture coordinates
	CO_FILTER_NEAREST = 0,
	/// Returns the weighted average of the four texture elements that are closest to the specified texture coordinates
	CO_FILTER_LINEAR = 1,
} CoFilter;


///
typedef enum
{
	/// 
	CO_WRAP_MODE_CLAMP_TO_EDGE = 0,
	/// 
	CO_WRAP_MODE_REPEAT = 1,
	/// 
	CO_WRAP_MODE_MIRROR = 2,
	/// 
	CO_WRAP_MODE_ONE = 3,
	/// 
	CO_WRAP_MODE_ZERO = 4,
} CoWrapMode;


///
typedef struct 
{
	/// 
	CoFilter minFilter;

	///
	CoFilter magFilter;

	///
	CoFilter mipmapFilter;

	///
	CoWrapMode wrapMode;

} CoSamplerCreateConfig;

struct CoSampler_T;

typedef CoSampler_T* CoSampler;

CORAL_API CoResult coContextCreateSampler(CoContext context, const CoSamplerCreateConfig* pConfig, CoSampler* pSampler);

CORAL_API  void coDestroySampler(CoSampler sampler);

CORAL_API CoFilter coSamplerGetMinFilter(const CoSampler sampler);

CORAL_API CoFilter coSamplerGetMagFilter(const CoSampler sampler);

CORAL_API CoFilter coSamplerGetMipMapFilter(const CoSampler sampler);

CORAL_API CoWrapMode coSamplerGetWrapMode(const CoSampler sampler);

#endif // !CORAL_SAMPLER_H
