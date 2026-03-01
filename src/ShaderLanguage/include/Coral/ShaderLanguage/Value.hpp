#ifndef CORAL_SHADERLANGUAGE_VALUE_HPP
#define CORAL_SHADERLANGUAGE_VALUE_HPP

#include <memory>

namespace Coral::ShaderLanguage
{

enum class ValueType
{
	/// Conditional type
	BOOL,
	/// 32-bit signed integer
	INT,
	/// 2-component 32-bit signed integer vector
	INT2,
	/// 3-component 32-bit signed integer vector
	INT3,
	/// 4-component 32-bit signed integer vector
	INT4,
	/// 32-bit single-precision floating-point number
	FLOAT,
	/// 2-component 32-bit single-precision floating-point vector
	FLOAT2,
	/// 3-component 32-bit single-precision floating-point vector
	FLOAT3,
	/// 4-component 32-bit single-precision floating-point vector
	FLOAT4,
	/// 3x3 32-bit single-precision floating-point matrix
	FLOAT3X3,
	/// 4x4 32-bit single-precision floating-point matrix
	FLOAT4X4,
	/// 2D texture sampler
	SAMPLER2D,

	STRUCT,
};


class Expression;
class ShaderGraph;

/// Base struct for all values of shader graph expressions
/**
 * Classes that derive from this type act as stack-allocated wrappers arround a shader graph expression and define the
 * output type of said operation. Classes of this type are widely used as inputs to functions that build up new shader
 * graph expressions. For example, the ´float32´ struct is a proxy for a 32-bit floating-point value that is created by
 * a shader graph expression. The object itself does not hold the actual value but stores a pointer to the shader graph
 * node from which it was calculated. The user is only required to interact with the stack-allocated Value objects and
 * function calls to build the shader graph. The creation and storage of the actual ShaderGraph is abstracted away via
 * this level of indirection.
 */
struct Value
{
public:

	Value();

	Value(std::shared_ptr<Expression> source);

	// Get the wrapped ShaderGraph node
	std::shared_ptr<Expression> source() const &;

	// Get the wrapped ShaderGraph node
	std::shared_ptr<Expression> source() &&;

	void setSource(std::shared_ptr<Expression> source);

	/// Get the type id of the value
	ValueType typeId() const;

private:

	std::shared_ptr<Expression> mSource;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_VALUE_HPP
