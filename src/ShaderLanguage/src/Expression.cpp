#include <Coral/ShaderLanguage/Expression.hpp>

#include <Coral/ShaderLanguage/ShaderModule.hpp>

#include <cassert>

using namespace Coral::ShaderLanguage;
using namespace Coral::ShaderLanguage;

Expression::Expression(ValueType resultType, const std::vector<ExpressionPtr>& inputs)
	: mOutputValueType(resultType)
	, mInputs(inputs)
{
}


ValueType
Expression::resultValueType() const
{ 
	return mOutputValueType;
}


std::vector<const Expression*>
Expression::inputs() const
{
	return mInputs
		| std::views::transform([](auto expr) { return expr.get(); })
		| std::ranges::to<std::vector<const Expression*>>();
}
