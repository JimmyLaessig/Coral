#ifndef CORAL_DESCRIPTORSET_H
#define CORAL_DESCRIPTORSET_H

#include <Coral/Context.h>
#include <Coral/Image.h>
#include <Coral/Buffer.h>
#include <Coral/Sampler.h>

inline const uint32_t CO_MAX_DESCRIPTOR_ARRAY_SIZE = -1;

/*!
 * Structure defining the layout of a descriptor
 */
typedef struct
{
    /*!
     * The binding index to which a descriptor matching the given type must be bound
     */
    uint32_t binding;

    /*!
     * The type of the descriptor
     */
    CoDescriptorType type;

    /*!
     * The number of elements in the descriptor. Must be larger than 0.
     * If the count is larger than 1, the descriptor is considered to be a ixed-size array with the
     * given number of elements. If count equals CO_MAX_DESCRIPTOR_ARRAY_SIZE the descriptor is 
     * considered an unbounded array where the size is not known at compile time.
     */
    uint32_t count;

} CoBindDescriptorInfo;

/*!
 * Structure defining the layout of a DescriptorSet
 */
typedef struct
{
    /*!
     * Pointer to an array of CoDescriptorInfo structures that describe the descriptor bindings of the Descriptor
     */
    const CoBindDescriptorInfo* pBindings;

    /*!
     * Number of elements in the pBindings array
     */
    uint32_t bindingsCount;

    /*!
     * The index of the DescriptorSet
     */
    uint32_t set;

} CoDescriptorSetLayout;

/*!
 * Structure specif
 */
typedef struct
{
    CoImage image;
    CoSampler sampler;

} CoCombinedImageSampler;

/*!
 *
 */
typedef struct
{
    /*!
     * The binding index to which the descriptor will be bound
     */
    uint32_t binding;

    /*!
     * The index of the element to set if the Descriptor is an array
     */
    uint32_t arrayIndex;

    /*!
     * The type of descriptor
     */
    CoDescriptorType type;

    union
    {
        /*!
         * Bind a buffer
         * \ref type must be one of:
         * - CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER 
         * - CO_DESCRIPTOR_TYPE_STORAGE_BUFFER
         */
        CoBuffer buffer;

        /*!
         * Bind an image
         * \ref type must be CO_DESCRIPTOR_TYPE_IMAGE
         */
        CoImage image;

        /*!
         * Bind a sampler
         * \ref type must be CO_DESCRIPTOR_TYPE_SAMPLER
         */
        CoSampler sampler;

        /*!
         * Bind an image and a sampler
         * \ref type must be CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
         */
        CoCombinedImageSampler combinedImageSampler;
    };

} CoBindDescriptor;

/*!
 * Structure specifying the parameters of a newly created DescriptorSet object
 */
typedef struct 
{
    /*!
     * Pointer to a \ref CoDescriptorSetLayout structure containing the layout of the DescriptorSet
     */
    const CoDescriptorSetLayout* pLayout;

    /*!
     * Pointer to an array of \ref CoBindDescriptor structures containing the descriptors to bind.
     */
    const CoBindDescriptor* pBindDescriptors;

    /*!
     * The number of elements in \ref pBindDescriptors
     */
    uint32_t bindDescriptorCount;

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
 * \brief Destroy the descriptorSet object
 * \param buffer Handle to a CoDescriptorSet object to destroy
 */
CORAL_API void coDestroyDescriptorSet(CoDescriptorSet descriptorSet);

#endif // !CORAL_DESCRIPTORSET_H
