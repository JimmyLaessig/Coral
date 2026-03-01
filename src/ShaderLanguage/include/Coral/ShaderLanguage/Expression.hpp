#ifndef CORAL_SHADERLANGUAGE_EXPRESSION_HPP
#define CORAL_SHADERLANGUAGE_EXPRESSION_HPP

#include <Coral/ShaderLanguage/Value.hpp>

#include <memory>
#include <vector>

namespace Coral::ShaderLanguage
{


// Unique type of each expression
enum class ExpressionType
{
	// The expression is a `ConstantExpression<float>`
	CONSTANT_FLOAT,
	// The expression is a `ConstantExpression<int>`
	CONSTANT_INT,
	// The expression is a `ConstantExpression<bool>`
	CONSTANT_BOOL,
	// The expression is an `InputAttributeExpression`
	INPUT_ATTRIBUTE,
	// The expression is an `OutputtAttributeExpression`
	OUTPUT_ATTRIBUTE,
	// The expression is a `UniformBufferExpression`
	UNIFORM_BUFFER,
	// The expression is a `UniformExpression`
	UNIFORM,
	// The expression is a `NativeFunctionExpression`
	NATIVE_FUNCTION,
	// The expression is a `ConstructorExpression`
	CONSTRUCTOR,
	// The expression is a `CastExpression`
	CAST,
	// The expression is a `SwizzleExpression`
	SWIZZLE,
	// The expression is an `OperatorExpression`
	OPERATOR,
	// The expression is a `SamplerExpression`
	SAMPLER,
};


class Expression;
using ExpressionPtr = std::shared_ptr<Expression>;

/// Base class for a shader graph expression
class Expression
{
public:

	Expression(ValueType valueType, const std::vector<ExpressionPtr>& inputs);

	virtual ~Expression() = default;

	// Get the unique type id of the expression
	virtual ExpressionType Type() const = 0;

	// Get the type of the output value of this expression
	ValueType GetValueType() const;

	// Get the list of inputs this expression
	std::vector<const Expression*> Inputs() const;

	/// Flag indicating if the expression should be inlined or requires an explicit variable assignment
	virtual bool InlineIfPossible() const
	{
		return mInline;
	}

	/// Mark the expression to be inlined if possible
	void SetInlineIfPossible()
	{
		mInline = true;
	}

	template<typename T>
	T* Cast()
	{
		return Type() == T::ClassType ? static_cast<T*>(this) : nullptr;
	}

	template<typename T>
	const T* Cast() const
	{
		return Type() == T::ClassType ? static_cast<const T*>(this) : nullptr;
	}

private:

	friend class ShaderGraph;

	std::vector<ExpressionPtr> mInputs;

	bool mInline{ false };

	ValueType mValueType;
};

} // namespace Coral::ShaderLanguage

#endif // !CORAL_SHADERLANGUAGE_EXPRESSION_HPP
