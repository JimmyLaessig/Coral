#include <Coral/BufferPool.hpp>

#include <memory>
#include <mutex>

using namespace Coral;


BufferPool::BufferPool(Context& context, CoBufferType bufferType, bool cpuVisible)
    : mContext(context)
    , mBufferType(bufferType)
    , mCpuVisible(mCpuVisible)
{
}


std::shared_ptr<Buffer>
BufferPool::requestBuffer(size_t bufferSize)
{
    std::lock_guard lock(mBufferPoolProtection);

    // Find the smallest staging buffer that fits the buffer size
    auto buffer = mBufferPool.end();
    for (auto iter = mBufferPool.begin(); iter != mBufferPool.end(); iter++)
    {
        if (iter->first >= bufferSize && iter->second.use_count() == 1)
        {
            buffer = iter;
        }
    }

    // If no staging buffer was found, create one that fits the buffer size
    if (buffer == mBufferPool.end())
    {
        CoBufferCreateConfig bufferConfig{};
        bufferConfig.cpuVisible = mCpuVisible;
        bufferConfig.type       = mBufferType;
        bufferConfig.size       = bufferSize;

        if (auto buf = mContext.createBuffer(bufferConfig))
        {
            buffer = mBufferPool.emplace(bufferConfig.size, buf.value());
        }
    }

    return buffer != mBufferPool.end() ? buffer->second: nullptr;
}
