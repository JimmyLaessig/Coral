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

	VOID
};

class Expression;

/// Base struct for all values of shader graph expressions
/**
 * Objects that derive from this type act as stack-allocated wrappers arround a shader graph operation and define the
 * output type of said operation. Objects of this type are widely used as inputs to functions that build up new shader
 * graph operations. For example, the ´Float´ struct is a proxy for a floating-point value that is created by a shader
 * graph operation. The object itself does not hold the actual value but stores a pointer to the shader graph node from
 * which it was calculated. The user is only required to interact with the stack-allocated ShaderGraphResult objects
 * and function calls to build the shader graph. The creation and storage of the actual ShaderGraph is abstracted away
 * by this level of indirection.
 */
struct Value
{
public:

	// Get the wrapped ShaderGraph node
	std::shared_ptr<Expression> source() const &;

	// Get the wrapped ShaderGraph node
	std::shared_ptr<Expression> source() &&;

	/// Get the type id of the value
	ValueType typeId() const;

	Value(std::shared_ptr<Expression> source);

	void setSource(std::shared_ptr<Expression> ex);

private:

	std::shared_ptr<Expression> mSource;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_VALUE_HPP
