#ifndef CORAL_SEMAPHORE_HPP
#define CORAL_SEMAPHORE_HPP

#include <Coral/System.hpp>

#include <cstdint>

namespace Coral
{

enum class SemaphoreCreationError
{
	INTERNAL_ERROR
};

class CORAL_API Semaphore
{
public:

	constexpr static uint64_t MAX_TIMEOUT = 0xFFFFFFFFFFFFFFFF;

	virtual ~Semaphore() = default;

	virtual bool wait(uint64_t timeout = MAX_TIMEOUT) = 0;

};


} // namespace Coral

#endif // !CORAL_SEMAPHORE_HPP