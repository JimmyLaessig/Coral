#include <Coral/BufferPool.hpp>

#include <memory>
#include <mutex>

using namespace Coral;


BufferPool::BufferPool(Coral::Context& context, Coral::BufferType bufferType, bool cpuVisible)
{
	mContext = &context;
	mBufferType = bufferType;
	mCpuVisible = cpuVisible;
}


Coral::BufferPtr
BufferPool::requestBuffer(size_t bufferSize)
{
	if (!mContext)
	{
		return nullptr;
	}

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
		Coral::BufferCreateConfig bufferConfig{};
		bufferConfig.cpuVisible = mCpuVisible;
		bufferConfig.type       = mBufferType;
		bufferConfig.size       = bufferSize;

		auto buf = mContext->createBuffer(bufferConfig).value_or(nullptr);

		if (buf)
		{
			buffer = mBufferPool.emplace(bufferConfig.size, buf);
		}
	}

	return buffer != mBufferPool.end() ? buffer->second: nullptr;
}
