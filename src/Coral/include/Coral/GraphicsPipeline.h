#ifndef CORAL_GRAPHICSPIPELINE_H
#define CORAL_GRAPHICSPIPELINE_H

#include <Coral/Export.h>

#include <Coral/DescriptorSet.h>
#include <Coral/ShaderModule.h>

#include <cstdint>

///
typedef enum
{
    CO_COMPARE_OP_NEVER,
    CO_COMPARE_OP_LESS,
    CO_COMPARE_OP_EQUAL,
    CO_COMPARE_OP_LESS_OR_EQUAL,
    CO_COMPARE_OP_GREATER,
    CO_COMPARE_OP_GREATER_OR_EQUAL,
    CO_COMPARE_OP_NOT_EQUAL,
    CO_COMPARE_OP_ALWAYS
} CoCompareOp;


/// Cull Mode
typedef enum
{
    CO_CULL_MODE_FRONT,
    CO_CULL_MODE_BACK,
    CO_CULL_MODE_FRONT_AND_BACK,
    CO_CULL_MODE_NONE,
} CoCullMode;


/// Front Face Orientation
typedef enum
{
    CO_FRONT_FACE_ORIENTATION_CW,
    CO_FRONT_FACE_ORIENTATION_CCW
} CoFrontFaceOrientation;


/// Face Culling Mode
typedef struct
{
    CoCullMode cullMode;
    CoFrontFaceOrientation orientation;
} CoFaceCullingMode;


/// Blend Factor
typedef enum 
{
    CO_BLEND_FACTOR_ZERO,
    CO_BLEND_FACTOR_ONE,
    CO_BLEND_FACTOR_SRC_ALPHA,
    CO_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    CO_BLEND_FACTOR_DST_ALPHA,
    CO_BLEND_FACTOR_ONE_MINUS_DST_ALPHA
} CoBlendFactor;


/// Blend Op
typedef enum 
{
    CO_BLEND_OP_ADD,
    CO_BLEND_OP_SUBTRACT,
    CO_BLEND_OP_REVERSE_SUBTRACT,
    CO_BLEND_OP_MIN,
    CO_BLEND_OP_MAX
} CoBlendOp;


typedef struct 
{
    CoBlendFactor srcFactor;
    CoBlendFactor destFactor;
    CoBlendOp blendOp;
} CoBlendMode;

// -----------------------------------------
//
//    Stencil Test
//
// -----------------------------------------

/// Stencil Op
typedef enum 
{
    CO_STENCIL_OP_KEEP,
    CO_STENCIL_OP_ZERO,
    CO_STENCIL_OP_REPLACE,
    CO_STENCIL_OP_INCREMENT,
    CO_STENCIL_OP_INCREMENT_WRAP,
    CO_STENCIL_OP_DECREMENT,
    CO_STENCIL_OP_DECREMENT_WRAP,
    CO_STENCIL_OP_INVERT
} CoStencilOp;


/// Stencil Op State
typedef struct 
{
    CoStencilOp failOp        ;
    CoStencilOp depthFailOp    ;
    CoStencilOp depthPassOp    ;
    CoCompareOp func        ;
    uint32_t ref            ;
    uint32_t mask            ;
} CoStencilOpState;


/// StencilTestMode
typedef struct 
{
    CoStencilOpState front;
    CoStencilOpState back;
} CoStencilTestMode;


/// Polygon offset
typedef struct 
{
    float factor;
    float units;
} CoPolygonOffset;


/// Depth test mode
typedef struct 
{
    bool writeDepth;
    CoCompareOp compareOp;
    CoPolygonOffset polygonOffset;
} CoDepthTestMode;


///
typedef enum 
{
    CO_POLYGON_MODE_SOLID,
    CO_POLYGON_MODE_WIREFRAME,
    CO_POLYGON_MODE_POINTS
} CoPolygonMode;


///
typedef struct 
{

} CoRasterizerMode;


///
typedef enum 
{
    CO_TOPOLOGY_POINT_LIST,
    CO_TOPOLOGY_TRIANGLE_LIST,
    CO_TOPOLOGY_LINE_LIST,
} CoTopology;

/*!
 * Structure specifying the attributes of a color attachment
 */
typedef struct
{
    /*!
     * Format of the attachment
     */
    CoPixelFormat format;

    /*!
     * Binding index of the attachment
     */
    uint32_t binding;

} CoColorAttachmentInfo;

/*!
 * Structure specifying the attributes of a depth-stencil attachment
 */
typedef struct
{
    /*!
     * Format of the attachment
     */
    CoPixelFormat format;

} CoDepthStencilAttachmentInfo;

/*!
 * Structure specifying the parameters of a newly created graphics pipeline object
 */
typedef struct 
{
    /*!
     * The shader module of the vertex stage
     */
    CoShaderModule vertexShaderModule;

    /*!
     * The shader module of the fragment stage
     */
    CoShaderModule fragmentShaderModule;

    /*!
     * Pointer to a list of CoColorAttachmentInfo structures
     */
    const CoColorAttachmentInfo* pColorAttachments;

    /*!
     * Number of elements in the pColorAttachments array
     */
    uint32_t colorAttachmentCount;

    /*!
     * Pointer to a CoDepthStencilAttachmentInfo instance. Can be nullptr.
     */
    const CoDepthStencilAttachmentInfo* depthStencilAttachment;

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
    /**
     * Default is no blending
     */
    CoBlendMode blendMode;

    /// The polygon mode of the pipeline
    /**
     * Default is SOLID.
     */
    CoPolygonMode polygonMode;

    ///
    CoTopology topology;
} CoGraphicsPipelineCreateConfig;

struct CoGraphicsPipeline_T;

typedef CoGraphicsPipeline_T* CoGraphicsPipeline;

/*!
 * \brief Create a new graphics pipeline object
 */
CORAL_API CoResult coContextCreateGraphicsPipeline(CoContext context, const CoGraphicsPipelineCreateConfig* pConfig, CoGraphicsPipeline* pGraphicsPipeline);

CORAL_API void coDestroyGraphicsPipeline(CoGraphicsPipeline pipeline);

CORAL_API uint32_t coGraphicsPipelineGetDescriptorSetLayoutCount(const CoGraphicsPipeline pipeline);

CORAL_API void coGraphicsPipelineGetDescriptorSetLayouts(const CoGraphicsPipeline pipeline, CoDescriptorSetLayout* pDescriptorSetLayouts);

#endif // !CORAL_GRAPHICSPIPELINE_H