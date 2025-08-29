#ifndef CORAL_BUFFERPOOL_HPP
#define CORAL_BUFFERPOOL_HPP

#include <Coral/Buffer.hpp>
#include <Coral/Context.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace Coral
{

class BufferPool
{
public:

	BufferPool(Coral::Context& context, Coral::BufferType bufferType, bool cpuVisible);

	/// Request a buffer from the pool
	/**
	 * The buffer will have at least the requested buffer size. Staging buffers must only be returned to the pool
	 * once any command buffers using the buffer has funished execution.
	 */
	Coral::BufferPtr requestBuffer(size_t bufferSize);

private:

	Coral::Context* mContext{ nullptr };

	std::multimap<size_t, Coral::BufferPtr> mBufferPool;

	std::mutex mBufferPoolProtection;

	Coral::BufferType mBufferType{ Coral::BufferType::STORAGE_BUFFER };

	bool mCpuVisible{ false };
}; 

} // namespace Coral

#endif // !CORAL_BUFFERPOOL_HPP