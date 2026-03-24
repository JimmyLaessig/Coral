#include <Coral/Context.h>
#include <Coral/Context.hpp>

#include <Coral/CommandQueue.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>

using namespace Coral;

CoResult
coCreateContext(const CoContextCreateConfig* config, CoContext* context)
{
    auto impl = Context::create(*config);
    if (impl)
    {
        *context = new CoContext_T{ impl.value() };
        return CO_SUCCESS;
    }

    return static_cast<CoResult>(impl.error());
}


void
coDestroyContext(CoContext context)
{
    delete context;
}


CoGraphicsAPI
coContextGetGraphicsAPI(const CoContext context)
{
    return context->impl->graphicsAPI();;
}


CoResult
coContextGetTransferQueue(const CoContext context, CoCommandQueue* pQueue)
{
    auto impl = context->impl->getTransferQueue();
    if (!impl)
    {
        return CO_FAILED;
    }

    auto iter = context->m_commandQueues.find(impl);
    if (iter == context->m_commandQueues.end())
    {
        iter = context->m_commandQueues.emplace(impl, std::make_unique<CoCommandQueue_T>(impl)).first;
    }

    *pQueue = iter->second.get();
    
    return CO_SUCCESS;;
}


CoResult
coContextGetGraphicsQueue(const CoContext context, CoCommandQueue* pQueue)
{
    auto impl = context->impl->getGraphicsQueue();

    if (!impl)
    {
        return CO_FAILED;
    }

    auto iter = context->m_commandQueues.find(impl);
    if (iter == context->m_commandQueues.end())
    {
        iter = context->m_commandQueues.emplace(impl, std::make_unique<CoCommandQueue_T>(impl)).first;
        iter->second->impl = impl;
    }

    *pQueue = iter->second.get();

    return CO_SUCCESS;;
}


CoResult
coContextGetComputeQueue(const CoContext context, CoCommandQueue* pQueue)
{
    auto impl = context->impl->getComputeQueue();

    if (!impl)
    {
        return CO_FAILED;
    }

    auto iter = context->m_commandQueues.find(impl);
    if (iter == context->m_commandQueues.end())
    {
        iter = context->m_commandQueues.emplace(impl, std::make_unique<CoCommandQueue_T>(impl)).first;
    }

    *pQueue = iter->second.get();

    return CO_SUCCESS;;
}


std::expected<std::shared_ptr<Context>, Context::CreateError>
Context::create(const Context::CreateConfig& config)
{
    if (config.graphicsAPI == CO_GRAPHICS_API_VULKAN)
    {
        return Vulkan::ContextImpl::create(config);
    }

    return std::unexpected{ Context::CreateError::UNSUPPORTED_GRAPHICS_API };
}
