#ifndef CORAL_GRAPHICSPIPELINE_HPP
#define CORAL_GRAPHICSPIPELINE_HPP

#include <Coral/GraphicsPipeline.h>

#include "CoralFwd.hpp"
#include "Framebuffer.hpp"

#include <vector>

namespace Coral
{

struct BindDescriptorInfo
{
    uint32_t binding{ 0 };
    uint32_t count{ 0 };
    CoDescriptorType type{ CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
};


struct DescriptorSetLayout
{
    uint32_t set{ 0 };
    std::vector<BindDescriptorInfo> bindings;
};


struct BindAttributeInfo
{
    uint32_t location{ 0 };
    CoAttributeFormat format{ CO_ATTRIBUTE_FORMAT_RGBA_F32 };
};


struct AttributeLayout
{
    std::vector<BindAttributeInfo> bindings;
};


/*!
 *
 */
class CORAL_API GraphicsPipeline
{
public:

    struct CreateConfig
    {
        std::vector<ShaderModulePtr> shaderModules;

        Framebuffer::Layout framebufferLayout;

        /*!
         * The face culling mode of the pipeline
         */
        CoFaceCullingMode faceCullingMode;

        /*!
         * The depth test mode of the pipeline
         */
        CoDepthTestMode depthTestMode;

        /*!
         * The stencil test mode of the pipeline. 
         */
         //StencilTestMode* stencilTestMode{ nullptr };

        /*!
         * The blend mode of the pipeline.
         */
        CoBlendMode blendMode;

        /*!
         * The polygon mode of the pipeline.
         */
        CoPolygonMode polygonMode;

        /*!
         * The topology of the pipeline.
         */
        CoTopology topology;
    };

    enum class CreateError
    {
        INTERNAL_ERROR
    };

    virtual ~GraphicsPipeline() = default;

    /*!
     * \brief Get the descriptor set layout of the pipeline
     */
    virtual const std::vector<DescriptorSetLayout>& getDescriptorSetLayouts() const = 0;

    /*!
     * \brief Get the attribute layout of the pipeline
     */
    virtual const AttributeLayout& getAttributeLayout() const = 0;
};

} // namespace Coral

struct CoGraphicsPipeline_T
{
    CoGraphicsPipeline_T(std::shared_ptr<Coral::GraphicsPipeline> impl);

    ~CoGraphicsPipeline_T();

    std::shared_ptr<Coral::GraphicsPipeline> impl;

    std::vector<CoDescriptorSetLayout> mDescriptorSetLayouts;
};

#endif // !CORAL_GRAPHICSPIPELINE_HPP
