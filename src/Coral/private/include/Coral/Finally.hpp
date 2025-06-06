#ifndef CORAL_FINALLY_HPP
#define CORAL_FINALLY_HPP

namespace Coral
{

/// Utility class that invokes the passed action on destruction
template<typename Action>
class Finally
{
public:

[[ nodiscard ]] explicit Finally(Action&& action)
		: mAction(action)
	{
	}

	~Finally()
	{
		mAction();
	}

	Finally(const Finally&) = delete;
	Finally(Finally&&) = delete;

	Finally& operator=(const Finally&) = delete;
	Finally& operator=(Finally&&) = delete;

private:

	Action mAction;
};

} // namespace Common

#endif // !CORAL_FINALLY_HPP
