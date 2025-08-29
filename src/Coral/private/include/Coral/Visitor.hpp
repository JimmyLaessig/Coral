#ifndef CORAL_VISITOR_HPP
#define CORAL_VISITOR_HPP

namespace Coral
{

// Visitor class definition for std::visit
template<class... Ts>
struct Visitor : Ts...
{
	using Ts::operator()...;
};

} // namespace Coral

#endif // !CORAL_VISITOR_HPP
