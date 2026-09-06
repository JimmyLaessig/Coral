#ifndef CORAL_GRAPHICSPIPELINE_HPP
#define CORAL_GRAPHICSPIPELINE_HPP

#include <Coral/GraphicsPipeline.h>

#include "CoralFwd.hpp"
#include "Framebuffer.hpp"

#include <vector>

namespace Coral
{

///
class CORAL_API GraphicsPipeline
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

    virtual ~GraphicsPipeline() = default;
};

} // namespace Coral

struct CoGraphicsPipeline_T
{
    std::shared_ptr<Coral::GraphicsPipeline> impl;
};

#endif // !CORAL_GRAPHICSPIPELINE_HPP
