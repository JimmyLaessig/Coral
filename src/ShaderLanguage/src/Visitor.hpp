#ifndef CORAL_SHADERGRAPH_VISITOR_HPP
#define CORAL_SHADERGRAPH_VISITOR_HPP

// Visitor class definition for std::visit
template<class... Ts>
struct Visitor : Ts...
{
	using Ts::operator()...;
};

#endif !CORAL_SHADERGRAPH_VISITOR_HPP