#ifndef CORAL_SHADERMODULE_H
#define CORAL_SHADERMODULE_H

#include <Coral/Context.h>

typedef enum
{
    CO_SHADER_STAGE_VERTEX   = 0,
    CO_SHADER_STAGE_FRAGMENT = 1,
} CoShaderStage;

/// Configuration to create a shader module object
typedef struct
{
    /// The name of the shader
    const char* pName;

    /// The stage of the shader
    CoShaderStage stage;

    /// The source code of the shader
    /**
     * \Note: Depending on the GraphicsAPI the shader source must be provided in the correct platform format:
     * * Vulkan: SpirV byte code in 4-byte words
     */
    const CoByte* pSource;

    uint32_t sourceCount;

    /// The name of the entry-point function of the shader
    const char* pEntryPoint;
} CoShaderModuleCreateConfig;


/// Structure specifying a shader attribute binding
typedef struct 
{
    // The location in the shader of the attribute
    uint32_t location;

    /// The format of the attribute
    CoAttributeFormat format;

    /// The name of the attribute in the shader
    const char* pName;
} CoAttributeBindingInfo;



/// Description of one member value of a struct
typedef struct 
{
    /// The type of the value
    CoUniformFormat type;
    
    /// The name of the value
    const char* pName;

    /// The number of elements in the member. A count > 1 defines an array with `count` values
    uint32_t count;

    /// The size in bytes of the member
    uint32_t size;
} CoMemberInfo;


typedef enum
{
    CO_UNIFORM_BUFFER_LAYOUT_STD_140,
    CO_UNIFORM_BUFFER_LAYOUT_STD_430,
} CoUniformBufferLayout;


/// Description of a uniform buffer
typedef struct 
{
    const CoMemberInfo* pMembers;

    uint32_t memberCount;

} CoUniformBlockDefinition;


/// Defines a uniform sampler descriptor
typedef struct 
{
} CoSamplerDefinition;


/// Defines a uniform texture descriptor
typedef struct 
{
} CoTextureDefinition;


/// Defines a uniform texture descriptor
typedef struct 
{
} CoCombinedTextureSamplerDefinition;


typedef enum
{
    CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER           = 0,
    CO_DESCRIPTOR_TYPE_TEXTURE                  = 1,
    CO_DESCRIPTOR_TYPE_SAMPLER                  = 2,
    CO_DESCRIPTOR_TYPE_COMBINED_TEXTURE_SAMPLER = 3,
} CoDescriptorType;


/// Structure specifying a shader descriptor binding
typedef struct 
{
    /// The binding index to which a descriptor matching this definition must be bound
    uint32_t binding;

    /// The name of the descriptor in the shader
    const char* pName;

    /// The type of the descriptor
    CoDescriptorType type;

    union
    {
        CoUniformBlockDefinition block;
        CoTextureDefinition texture;
        CoSamplerDefinition sampler;
        CoCombinedTextureSamplerDefinition combinedTextureSampler;
    };

} CoDescriptorBindingInfo;

typedef struct
{
    const CoDescriptorBindingInfo* pDescriptorBindingInfos;

    uint32_t descriptorBindingInfoCount;

    const CoAttributeBindingInfo* pInputAttributeBindingInfos;

    uint32_t inputAttributeBindingInfoCount;

    const CoAttributeBindingInfo* pOutputAttributeBindingInfos;

    uint32_t outputAttributeBindingInfoCount;

} CoShaderModuleLayout;


struct CoShaderModule_T;

typedef CoShaderModule_T* CoShaderModule;

CORAL_API CoResult coContextCreateShaderModule(CoContext context, 
                                               const CoShaderModuleCreateConfig* pConfig, 
                                               CoShaderModule* pShaderModule);

CORAL_API void coDestroyShaderModule(CoShaderModule shaderModule);

/// Get the layout of the shader module
CORAL_API void coShaderModuleGetLayout(const CoShaderModule shaderModue, CoShaderModuleLayout* pLayout);

#endif // !CORAL_SHADERMODULE_H
