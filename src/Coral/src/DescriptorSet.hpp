#ifndef CORAL_DESCRIPTORSET_HPP
#define CORAL_DESCRIPTORSET_HPP

#include <Coral/DescriptorSet.h>
#include "CoralFwd.hpp"

#include <expected>
#include <vector>

namespace Coral
{

/*!
 * A Command Queue executes submitted work
 */
class CORAL_API DescriptorSet
{
public:

    using CreateConfig = CoDescriptorSetCreateConfig;

    enum class CreateError
    {
        INTERNAL_ERROR
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
