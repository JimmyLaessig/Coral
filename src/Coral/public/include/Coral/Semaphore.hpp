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

	virtual ~Semaphore() = default;

};


} // namespace Coral

#endif // !CORAL_SEMAPHORE_HPP