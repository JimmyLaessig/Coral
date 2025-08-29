#include <Coral/Vulkan/Resource.hpp>

using namespace Coral::Vulkan;

Resource::Resource(ContextImpl& context)
	: mContext(context)
{
}


ContextImpl&
Resource::context()
{
	return mContext;
}
