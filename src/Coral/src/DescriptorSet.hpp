#ifndef CORAL_DESCRIPTORSET_HPP
#define CORAL_DESCRIPTORSET_HPP

#include <Coral/DescriptorSet.h>
#include "CoralFwd.hpp"

#include "GraphicsPipeline.hpp"

#include <expected>
#include <vector>
#include <variant>

namespace Coral
{

struct CombinedImageSampler
{
    std::shared_ptr<Image> image;
    std::shared_ptr<Sampler> sampler;
};

struct BindDescriptor
{
    uint32_t binding{ 0 };

    std::variant<std::shared_ptr<Buffer>,
                 std::shared_ptr<Image>,
                 std::shared_ptr<Sampler>,
                 CombinedImageSampler> descriptor;
};

/*!
 * 
 */
class CORAL_API DescriptorSet
{
public:

    struct CreateConfig
    {
        std::vector<BindDescriptor> bindings;

        DescriptorSetLayout layout;
    };

    enum class CreateError
    {
        INTERNAL_ERROR    = CO_ERROR_INTERNAL,
        INVALID_ARGUMENT  = CO_ERROR_INVALID_ARGUMENT,
    };

    virtual ~DescriptorSet() = default;

}; // class DescriptorSet

} // namespace Coral

/*!
 *
 */
struct CoDescriptorSet_T
{
    Coral::DescriptorSetPtr impl;

}; // struct CoDescriptorSet_T

#endif // !CORAL_DESCRIPTORSET_HPP
