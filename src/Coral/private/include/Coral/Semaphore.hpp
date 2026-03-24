#ifndef CORAL_SEMAPHORE_HPP
#define CORAL_SEMAPHORE_HPP

#include <Coral/Semaphore.h>

#include <memory>

namespace Coral
{

class CORAL_API Semaphore
{
public:
	using CreateConfig = CoSemaphoreCreateConfig;

	enum class CreateError
	{
		INTERNAL_ERROR
	};

	virtual ~Semaphore() = default;

};

} // namespace Coral

struct CoSemaphore_T
{
	std::shared_ptr<Coral::Semaphore> impl;
};


#endif // !CORAL_SEMAPHORE_HPP