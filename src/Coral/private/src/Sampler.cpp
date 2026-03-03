#include <Coral/Sampler.h>
#include <Coral/Sampler.hpp>
#include <Coral/Context.hpp>

using namespace Coral;


CoResult
coContextCreateSampler(CoContext context, const CoSamplerCreateConfig* pConfig, CoSampler* pSampler)
{
    if (auto impl = context->impl->createSampler(*pConfig))
    {
        *pSampler = new CoSampler_T{ impl.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroySampler(CoSampler sampler)
{
    delete sampler;
}


CoFilter
coSamplerGetMinFilter(const CoSampler sampler)
{
    return sampler->impl->minFilter();
}


CoFilter
coSamplerGetMagFilter(const CoSampler sampler)
{
    return sampler->impl->magFilter();
}


CoFilter
coSamplerGetMipMapFilter(const CoSampler sampler)
{
    return sampler->impl->mipMapFilter();
}


CoWrapMode
coSamplerGetWrapMode(const CoSampler sampler)
{
    return sampler->impl->wrapMode();
}
