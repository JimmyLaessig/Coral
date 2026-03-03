#ifndef CORAL_PIPELINESTATE_HPP
#define CORAL_PIPELINESTATE_HPP

#include <Coral/PipelineState.h>
#include <Coral/CoralFwd.hpp>
#include <Coral/Framebuffer.hpp>

#include <cstdint>

#include <vector>


namespace Coral
{

///
class CORAL_API PipelineState
{
public:

	struct CreateConfig
	{
		std::vector<ShaderModulePtr> shaderModules;

		Framebuffer::Layout framebufferLayout;

		/// The face culling mode of the pipeline 
		/**
		 * Default is back-face culling (front faces are CCW)
		 */
		CoFaceCullingMode faceCullingMode;

		/// The depth test mode of the pipeline. 
		/**
		 * Default is depth test enabled with LESS_OR_EQUAL compare op.
		 */
		CoDepthTestMode depthTestMode;

		/// The stencil test mode of the pipeline. 
		/**
		 * If unset, no stencil test is performed.
		 */
		 //StencilTestMode* stencilTestMode{ nullptr };

		 /// The blend mode of the pipeline. 
		CoBlendMode blendMode;

		/// The polygon mode of the pipeline

		CoPolygonMode polygonMode;

		///
		CoTopology topology;
	};

	enum class CreateError
	{
		INTERNAL_ERROR
	};

	virtual ~PipelineState() = default;
};

} // namespace Coral

struct CoPipelineState_T
{
	std::shared_ptr<Coral::PipelineState> impl;
};

#endif // !CORAL_PIPELINESTATE_HPP