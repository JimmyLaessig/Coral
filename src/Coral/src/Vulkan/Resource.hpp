#ifndef CORAL_VULKAN_RESOURCE_HPP
#define CORAL_VULKAN_RESOURCE_HPP

#include "ContextImpl.hpp"

namespace Coral::Vulkan
{

/*!
 * Base class of a Vulkan Resource
 */
class Resource
{
public:

    Resource(ContextImpl& context);

    ContextImpl& context();

private:

    ContextImpl& mContext;

};  // class Resource

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_RESOURCE_HPP