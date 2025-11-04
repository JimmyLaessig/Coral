#ifndef CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
#define CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP

#include <Coral/ShaderLanguage/Expression.hpp>
#include <Coral/ShaderLanguage/Variable.hpp>
#include <concepts>

namespace Coral::ShaderLanguage
{

template<std::size_t N>
struct StringLiteral
{
	char value[N];

	constexpr StringLiteral(const char(&str)[N])
	{
		std::copy_n(str, N, value);
	}
};

namespace DefaultAttributes
{
	/// Set the position of a vertex in homogenous space. Every vertex shader must write out a parameter with this output.
	/*
	 * \Note: The position output value must be a 4-component float vector.
	 * \Note: The POSITION semantic is only available in vertex shaders.
	 */
	constexpr const auto POSITION = StringLiteral("POSITION");

	/// Shader output that is used to override the z buffer value in the fragment shader.
	/*
	 * \Note: The depth output value must be a single float.
	 * \Note: the DEPTH semantic is only in fragment shaders.
	 */
	constexpr const auto DEPTH    = StringLiteral("DEPTH");
} // namespace DefaultAttributes


template<typename T, StringLiteral Name>
struct InAttribute final : public T
{
	InAttribute()
		: T(ShaderModule::current()->addExpression<InputAttributeExpression>(T::toShaderTypeId(), Name.value))
	{
		
	}

	InAttribute(const InAttribute&) = delete;
	InAttribute(InAttribute&&)      = delete;
};


template<typename T, StringLiteral Name>
struct OutAttribute final : public T
{
	OutAttribute()
		: T(ShaderModule::current()->addExpression<OutputAttributeExpression>(T::toShaderTypeId(), Name.value))
	{
	}

	OutAttribute(const OutAttribute&) = delete;
	OutAttribute(OutAttribute&&) = delete;

	OutAttribute& operator=(const T& other)
	{
		ShaderModule::current()->addExpression<OperatorExpression>(T::toShaderTypeId(), T::source(), Operator::ASSIGNMENT, other.source());
		return *this;
	}

	OutAttribute& operator=(T&& other)
	{
		ShaderModule::current()->addExpression<OperatorExpression>(T::toShaderTypeId(), T::source(), Operator::ASSIGNMENT, other.source());
		return *this;
	}
};


template<typename T, StringLiteral Name>
struct Parameter final : public T
{
	Parameter()
		: T(ShaderModule::current()->addExpression<ParameterExpression>(T::toShaderTypeId(), Name.value))
	{
	}

	Parameter(const Parameter&) = delete;
	Parameter(Parameter&&) = delete;
	Parameter& operator=(const Parameter&) = delete;
	Parameter& operator=(Parameter&&) = delete;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
