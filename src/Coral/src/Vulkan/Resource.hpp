#ifndef CORAL_VULKAN_RESOURCE_HPP
#define CORAL_VULKAN_RESOURCE_HPP

#include "Vulkan.hpp"

namespace Coral::Vulkan
{
class ContextImpl;

/*!
 * Base class of a Vulkan Resource
 */
class Resource
{
public:

    Resource(ContextImpl& context);

    ContextImpl& context();

    const ContextImpl& context() const;

private:

    ContextImpl& mContext;

};  // class Resource

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_RESOURCE_HPP