#ifndef CORAL_DESCRIPTORSET_H
#define CORAL_DESCRIPTORSET_H

#include <Coral/Context.h>
#include <Coral/Image.h>
#include <Coral/Buffer.h>
#include <Coral/Sampler.h>

typedef struct
{
    CoImage image;
    CoSampler sampler;

} CoCombinedImageSampler;

typedef struct
{
    /*!
     * The binding index to which the descriptor will be bound
     */
    uint32_t binding;

    /*!
     * The type of descriptor to bind
     */
    CoDescriptorType type;

    union
    {
        CoBuffer buffer;
        CoImage image;
        CoSampler sampler;
        CoCombinedImageSampler combinedImageSampler;
    };

} CoDescriptorBinding;

/*!
 * Structure specifying the parameters of a newly created DescriptorSet object
 */
typedef struct 
{
    /*!
     * Pointer to an array of \ref CoDescriptorBinding structures containing the descriptors to bind.
     */
    CoDescriptorBinding* pDescriptorBindings;

    /*!
     * The number of elements in \ref pDescriptorBindings
     */
    uint32_t descriptorBindingCount;

} CoDescriptorSetCreateConfig;

struct CoDescriptorSet_T;

typedef CoDescriptorSet_T* CoDescriptorSet;

/*!
 * \brief Create a DescriptorSet object
 * \param context Handle to a CoContext object that creates the DescriptorSet object.
 * \param pConfig Pointer to a CoDescriptorSetCreateConfig instance containing parameters affecting the DescriptorSet
 *                creation.
 * \param[out] pDescriptorSet Pointer to a CoDescriptorSet handle in which the resulting DescriptorSet object is 
 *                            returned.
 */
CORAL_API CoResult coContextCreateDescriptorSet(CoContext context,
                                                const CoDescriptorSetCreateConfig* pConfig,
                                                CoDescriptorSet* pDescriptorSet);

/*!
 * \brief Destroy the DescriptorSet object
 * \param descriptorSet Handle to the CoDescriptorSet object to destroy
 */
CORAL_API void coDestroyDescriptorSet(CoDescriptorSet descriptorSet);

#endif // !CORAL_DESCRIPTORSET_H
