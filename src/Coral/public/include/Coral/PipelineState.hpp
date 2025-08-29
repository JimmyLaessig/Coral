#ifndef CORAL_PIPELINESTATE_HPP
#define CORAL_PIPELINESTATE_HPP

#include <Coral/System.hpp>
#include <Coral/CoralFwd.hpp>
#include <Coral/Framebuffer.hpp>

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>


namespace Coral
{

///
enum class CompareOp
{
	NEVER,
	LESS,
	EQUAL,
	LESS_OR_EQUAL,
	GREATER,
	GREATER_OR_EQUAL,
	NOT_EQUAL,
	ALWAYS
};


/// Cull Mode
enum class CullMode
{
	FRONT,
	BACK,
	FRONT_AND_BACK,
	NONE,
};


/// Front Face Orientation
enum class FrontFaceOrientation
{
	CW,
	CCW
};


/// Face Culling Mode
struct FaceCullingMode
{
	CullMode cullMode{ CullMode::BACK };
	FrontFaceOrientation orientation{ FrontFaceOrientation::CCW };
};


namespace FaceCullingModes
{
	/// Default FaceCullingMode for back face culling
	constexpr FaceCullingMode BackFaceCulling = { CullMode::BACK, FrontFaceOrientation::CCW };

	/// Default FaceCullingMode for front face culling
	constexpr FaceCullingMode FrontFaceCulling = { CullMode::FRONT, FrontFaceOrientation::CCW };

	/// Default FaceCullingMode to disable face culling
	constexpr FaceCullingMode None = { CullMode::NONE, FrontFaceOrientation::CCW };

} // namspace FaceCullingModes


/// Blend Factor
enum class BlendFactor
{
	ZERO,
	ONE,
	SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA,
	DST_ALPHA,
	ONE_MINUS_DST_ALPHA
};


/// Blend Op
enum class BlendOp
{
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX
};


struct BlendMode
{
	BlendFactor srcFactor{ BlendFactor::ONE };
	BlendFactor destFactor{ BlendFactor::ONE };
	BlendOp blendOp{ BlendOp::ADD };
};


namespace BlendModes
{
	/// Default Blend mode for alpha blending
	constexpr BlendMode Blend = { BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD };

	/// Default Blend mode for additive blending
	constexpr BlendMode Additive = { BlendFactor::ONE, BlendFactor::ONE, BlendOp::ADD };

	/// Default Blend mode for no blending
	constexpr BlendMode None = { BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD };

} // namespace BlendModes

// -----------------------------------------
//
//    Stencil Test
//
// -----------------------------------------

/// Stencil Op
enum class StencilOp
{
	KEEP,
	ZERO,
	REPLACE,
	INCREMENT,
	INCREMENT_WRAP,
	DECREMENT,
	DECREMENT_WRAP,
	INVERT
};


/// Stencil Op State
struct StencilOpState
{
	StencilOp failOp		{ StencilOp::KEEP	};
	StencilOp depthFailOp	{ StencilOp::KEEP	};
	StencilOp depthPassOp	{ StencilOp::KEEP	};
	CompareOp func			{ CompareOp::ALWAYS	};
	uint32_t ref			{ 0					};
	uint32_t mask			{ 0xFFFFFFFF		};
};


/// StencilTestMode
struct StencilTestMode
{
	StencilOpState front;
	StencilOpState back;
};


/// Polygon offset
struct PolygonOffset
{
	float factor{ 0.0f };
	float units { 0.0f };
};


/// Depth test mode
struct DepthTestMode
{
	bool writeDepth { true };
	CompareOp compareOp	{ CompareOp::LESS };
	PolygonOffset polygonOffset;
};


namespace DepthTestModes
{
	/// Default DepthTestMode to disable depth test
	constexpr DepthTestMode None = { true, Coral::CompareOp::ALWAYS, {} };

	/// Default DepthTestMode to enable depth test with LESS comparison
	constexpr DepthTestMode Less = { true, Coral::CompareOp::LESS, {} };

	/// Default DepthTestMode to enable depth test with LESS_OR_EQUAL comparison
	constexpr DepthTestMode LessOrEqual = { true, Coral::CompareOp::LESS_OR_EQUAL, {} };

} // namespace DepthTestModes


///
enum class PolygonMode
{
	SOLID,
	WIREFRAME,
	POINTS
};


///
struct RasterizerMode
{

};


///
enum class Topology
{
	POINT_LIST,
	TRIANGLE_LIST,
	LINE_LIST,
};


///
struct PipelineStateCreateConfig
{
	std::span<ShaderModule*> shaderModules;

	FramebufferSignature framebufferSignature;

	/// The face culling mode of the pipeline 
	/**
	 * Default is back-face culling (front faces are CCW)
	 */
	FaceCullingMode faceCullingMode = FaceCullingModes::BackFaceCulling;

	/// The depth test mode of the pipeline. 
	/**
	 * Default is depth test enabled with LESS_OR_EQUAL compare op.
	 */
	DepthTestMode depthTestMode = DepthTestModes::LessOrEqual;

	/// The stencil test mode of the pipeline. 
	/**
	 * If unset, no stencil test is performed.
	 */
	//StencilTestMode* stencilTestMode{ nullptr };

	/// The blend mode of the pipeline. 
	/**
	 * Default is no blending
	 */
	BlendMode blendMode = BlendModes::None;

	/// The polygon mode of the pipeline
	/**
	 * Default is SOLID.
	 */
	PolygonMode polygonMode = PolygonMode::SOLID;

	///
	Topology topology{ Topology::TRIANGLE_LIST };
};

enum class PipelineStateCreationError
{
	INTERNAL_ERROR
};

///
class CORAL_API PipelineState
{
public:

	virtual ~PipelineState() = default;
};



} // namespace Coral

#endif // !CORAL_PIPELINESTATE_HPP