#include <Coral/DescriptorSet.h>

#include "Context.hpp"

#include "DescriptorSet.hpp"

using namespace Coral;

namespace
{

std::optional<BindDescriptor>
convert(const CoBindDescriptor& d)
{
    switch (d.type)
    {
    case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        if (!d.buffer)
        {
            return std::nullopt;
        }
        return BindDescriptor{ 
            .binding = d.binding,            
            .descriptor = d.buffer->impl
        };
    case CO_DESCRIPTOR_TYPE_IMAGE:
        if (!d.image)
        {
            return std::nullopt;
        }
        return BindDescriptor{ 
            .binding    = d.binding,              
            .descriptor = d.image->impl
        };
    case CO_DESCRIPTOR_TYPE_SAMPLER:
        if (!d.sampler)
        {
            return std::nullopt;
        }
        return BindDescriptor{ 
            .binding    = d.binding,               
            .descriptor = d.sampler->impl 
        };

    case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        if (!d.combinedImageSampler.image ||
            !d.combinedImageSampler.sampler)
        {
            return std::nullopt;
        }

        return BindDescriptor{ 
            .binding     = d.binding, 
            .descriptor  = CombinedImageSampler{ 
                .image   = d.combinedImageSampler.image->impl,  
                .sampler = d.combinedImageSampler.sampler->impl
            }
        };

    default:
        return std::nullopt;
    }
}


std::optional<std::vector<BindDescriptor>>
convert(std::span<const CoBindDescriptor> descriptors)
{
    std::vector<BindDescriptor> result;
    for (const auto& binding : descriptors)
    {
        if (auto b = convert(binding))
        {
            result.push_back(*b);
        }
        else
        {
            return std::nullopt;
        }
    }

    return { std::move(result) };
}


Coral::BindDescriptorInfo
convert(const CoBindDescriptorInfo& info)
{
    return Coral::BindDescriptorInfo
    {
        .binding = info.binding,
        .count   = info.count,
        .type    = info.type,
    };
}


DescriptorSetLayout
convert(const CoDescriptorSetLayout& layout)
{
    DescriptorSetLayout result{};
    result.set = layout.set;

    result.bindings.reserve(layout.bindingsCount);
    for (const auto& binding : std::span(layout.pBindings, layout.bindingsCount))
    {
        result.bindings.push_back(convert(binding));
    }

    return result;
}

} // namespace

CoResult
coContextCreateDescriptorSet(CoContext context,
                             const CoDescriptorSetCreateConfig* pConfig,
                             CoDescriptorSet* pDescriptorSet)
{
    if (!pConfig)
    {
        return CO_ERROR_INVALID_ARGUMENT;
    }

    if (!pConfig->pLayout)
    {
        return CO_ERROR_INVALID_ARGUMENT;
    }

    if (pConfig->bindDescriptorCount > 0 && pConfig->pBindDescriptors == nullptr)
    {
        return CO_ERROR_INVALID_ARGUMENT;
    }

    DescriptorSet::CreateConfig config{};

    if (auto bindings = convert(std::span(pConfig->pBindDescriptors, pConfig->bindDescriptorCount)))
    {
        config.bindings = std::move(*bindings);
    }
    else
    {
        return CO_ERROR_INVALID_ARGUMENT;
    }

    config.layout = convert(*pConfig->pLayout);

    if (auto result = context->impl->createDescriptorSet(config))
    {
        *pDescriptorSet = new CoDescriptorSet_T{ result.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(result.error());
    }
}


void
coDestroyDescriptorSet(CoDescriptorSet descriptorSet)
{
    delete descriptorSet;
}
