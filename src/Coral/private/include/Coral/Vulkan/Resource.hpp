#ifndef CORAL_VULKAN_RESOURCE_HPP
#define CORAL_VULKAN_RESOURCE_HPP

#include <Coral/Vulkan/ContextImpl.hpp>

namespace Coral::Vulkan
{

class Resource
{
public:

	Resource(ContextImpl& context);

	ContextImpl& context();

private:

	ContextImpl& mContext;

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_RESOURCE_HPP