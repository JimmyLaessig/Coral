#ifndef CORAL_VISITOR_HPP
#define CORAL_VISITOR_HPP

#include <variant>

 namespace Coral
{

// Visitor class definition for std::visit
template<class... Ts>
struct Visitor : Ts...
{
	using Ts::operator()...;
};


template<typename Variant, typename Result, typename ...Ts>
Result visit(Variant variant, Visitor<Ts...>&& visitor)
{
	return std::visit(visitor, variant);
}

} // namespace Coral

#endif // !CORAL_VISITOR_HPP
