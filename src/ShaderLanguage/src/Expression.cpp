#include <Coral/ShaderLanguage/Expression.hpp>

#include <ranges>

using namespace Coral::ShaderLanguage;

Expression::Expression(ValueType resultType, const std::vector<ExpressionPtr>& inputs)
	: mValueType(resultType)
	, mInputs(inputs)
{
}


ValueType
Expression::GetValueType() const
{ 
	return mValueType;
}


std::vector<const Expression*>
Expression::Inputs() const
{
	return mInputs
		| std::views::transform([](auto expr) { return static_cast <const Expression*>(expr.get()); })
		| std::ranges::to<std::vector<const Expression*>>();

	return {};
}
