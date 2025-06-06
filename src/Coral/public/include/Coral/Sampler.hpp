#ifndef CORAL_SAMPLER_HPP
#define CORAL_SAMPLER_HPP

#include <Coral/System.hpp>

namespace Coral
{

/// 
enum class Filter
{
	/// Return the texture element that is nearest to the specified texture coordinates
	NEAREST,
	/// Returns the weighted average of the four texture elements that are closest to the specified texture coordinates
	LINEAR
};


///
enum class WrapMode
{
	/// 
	CLAMP_TO_EDGE,
	/// 
	REPEAT,
	/// 
	MIRROR,
	/// 
	ONE,
	/// 
	ZERO,
};


///
struct SamplerCreateConfig
{
	/// 
	Filter minFilter{ Filter::LINEAR };

	///
	Filter magFilter{ Filter::LINEAR };

	///
	Filter mipmapFilter{ Filter::LINEAR };

	///
	WrapMode wrapMode{ WrapMode::REPEAT };
};


enum class SamplerCreationError
{
	INTERNAL_ERROR
};

/// 
class CORAL_API Sampler
{
public:

	virtual ~Sampler() = default;

	///
	virtual Filter magFilter() const = 0;

	///
	virtual Filter minFilter() const = 0;

	///
	virtual Filter mipMapFilter() const = 0;

	///
	virtual WrapMode wrapMode() const = 0;
};

} // namespace Coral

#endif // !CORAL_SAMPLER_HPP
