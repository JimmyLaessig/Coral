#ifndef CORAL_SHADERMODULE_H
#define CORAL_SHADERMODULE_H

#include <Coral/Context.h>

/*!
 * Enum specifying the stage of a shader
 */
typedef enum
{
    CO_SHADER_STAGE_VERTEX   = 0,
    CO_SHADER_STAGE_FRAGMENT = 1,
} CoShaderStage;

/*!
 * Configuration to create a shader module object
 */
typedef struct
{
    /*!
     * The name of the shader
     */
    const char* pName;

    /*!
     * The stage of the shader
     */
    CoShaderStage stage;

    /*!
     * The source code of the shader
     * \Note: Depending on the GraphicsAPI the shader source must be provided in the correct platform format:
     * * Vulkan: SpirV byte code in 4-byte words
     */
    const CoByte* pSource;

    /*!
     * The number of bytes in the
     */
    uint32_t sourceCount;

    /*!
     * The name of the entry point function of the shader
     */
    const char* pEntryPoint;

} CoShaderModuleCreateConfig;

/*!
 * \brief A shader module maintains the source code of a shader and its reflection data. 
 * A shader specifies the programmable operations of each stage of a graphics or compute pipeline.
 * A shader module is created from a shader source code and can be used to create a pipeline
 * object.
 */
struct CoShaderModule_T;

typedef CoShaderModule_T* CoShaderModule;

/*!
 * \brief Create a shader module object
 * \param context Handle to a CoContext object that creates the shader module object
 * \param pConfig Pointer to a CoShaderModuleCreateConfig instance containing parameters affecting the shader module creation
 * \param[out] pShaderModule Pointer to a CoShaderModule handle in which the resulting shader module object is returned
 * \return Returns CO_SUCCESS if shader module creation suceeded. The following errror codes may be returned:
 *         CO_INVALID_ARGUMENT if context, pConfig or pShaderModule are null pointers. 
 *         CO_INVALID_ARGUMENT if pName, pEntryPoint or pSource of pConfig are null pointers.
 *         CO_INVALID_ARGUMENT if sourceCount of pConfig is 0.
 */
CORAL_API CoResult coContextCreateShaderModule(CoContext context, 
                                               const CoShaderModuleCreateConfig* pConfig, 
                                               CoShaderModule* pShaderModule);

/*!
 * \brief Destroy the shader module object
 * \param shaderModule Handle to a CoShaderModule object to destroy
 */
CORAL_API void coDestroyShaderModule(CoShaderModule shaderModule);

#endif // !CORAL_SHADERMODULE_H
