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


std::shared_ptr<Coral::Buffer>
BufferPool::requestBuffer(size_t bufferSize)
{
	if (!mContext)
	{
		return nullptr;
	}

	std::lock_guard lock(mBufferPoolProtection);

	// Find the smallest staging buffer that fits the buffer size
	auto candidate = mBufferPool.end();
	for (auto iter = mBufferPool.begin(); iter != mBufferPool.end(); iter++)
	{
		if (iter->first >= bufferSize && iter->second.use_count() == 1)
		{
			candidate = iter;
		}
	}

	// If no staging buffer was found, create one that fits the buffer size
	if (candidate != mBufferPool.end())
	{
		auto buffer = std::move(candidate->second);
		mBufferPool.erase(candidate);
	}

	Coral::BufferCreateConfig bufferConfig{};
	bufferConfig.cpuVisible = mCpuVisible;
	bufferConfig.type		= mBufferType;
	bufferConfig.size		= bufferSize;

	return Coral::BufferPtr(mContext->createBuffer(bufferConfig).value_or(nullptr));
}
