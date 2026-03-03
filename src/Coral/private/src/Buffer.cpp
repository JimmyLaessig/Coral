#include <Coral/Buffer.h>
#include <Coral/Buffer.hpp>
#include <Coral/Context.hpp>

#include <memory>
#include <mutex>

using namespace Coral;

CoResult
coContextCreateBuffer(CoContext context, const CoBufferCreateConfig* pConfig, CoBuffer* pBuffer)
{
    if (auto impl = context->impl->createBuffer(*pConfig))
    {
        *pBuffer = new CoBuffer_T{ impl.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroyBuffer(CoBuffer buffer)
{
    delete buffer;
}


uint64_t
coBufferGetSize(const CoBuffer buffer)
{
    return buffer->impl->size();
}


CoBufferType
coBufferGetType(const CoBuffer buffer)
{
    return buffer->impl->type();
}


CoResult
coBufferMap(CoBuffer buffer, CoByte** pBytes)
{
    if (auto mapped = buffer->impl->map())
    {
        *pBytes = (CoByte*)(mapped);
        return CO_SUCCESS;
    }
    return CO_FAILED;
}


CoResult
coBufferUnMap(CoBuffer buffer)
{
    return buffer->impl->unmap() ? CO_SUCCESS : CO_FAILED;
}
