#include <Coral/DescriptorSet.h>

#include "Context.hpp"

#include "DescriptorSet.hpp"

using namespace Coral;


CoResult coContextCreateDescriptorSet(CoContext context,
                                      const CoDescriptorSetCreateConfig* pConfig,
                                      CoDescriptorSet* pDescriptorSet)
{
    if (auto result = context->impl->createDescriptorSet(*pConfig))
    {
        *pDescriptorSet = new CoDescriptorSet_T{ result.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(result.error());
    }
}


void coDestroyDescriptorSet(CoDescriptorSet descriptorSet)
{
    delete descriptorSet;
}