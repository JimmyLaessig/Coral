#ifndef CORAL_FENCE_HPP
#define CORAL_FENCE_HPP

#include <Coral/System.hpp>

namespace Coral
{

enum class FenceCreationError
{
	INTERNAL_ERROR
};

// A fence is a GPU-CPU synchronization mechanic
class CORAL_API Fence
{
public:

	virtual ~Fence() = default;

	virtual bool wait() = 0;

	virtual void reset() = 0;
};



} // namespace Coral

#endif // !CORAL_FENCE_HPP