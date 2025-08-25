#ifndef CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
#define CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP

#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <Coral/ShaderLanguage/Scalar.hpp>

namespace Coral::ShaderLanguage
{

template<typename T>
struct Input : public T
{
	Input(std::string_view name)
		: T(std::make_shared<ShaderGraph::InputAttributeExpression>(T::toShaderTypeId(), name))
	{
	}

	Input(const Input&)            = delete;
	Input(Input&&)                 = delete;
	Input& operator=(const Input&) = delete;
	Input& operator=(Input&&)      = delete;
};


using DefaultAttribute = ShaderGraph::DefaultAttribute;


template<typename T>
struct Output final: public Value
{
	Output(std::string_view name, const T& value)
		: Value(std::make_shared<ShaderGraph::OutputAttributeExpression>(name, value.source()))
	{
	}

	Output(DefaultAttribute attribute, const T& value)
		: Value(std::make_shared<ShaderGraph::OutputAttributeExpression>(attribute, value.source()))
	{
	}

	Output(const Output&)            = delete;
	Output(Output&&)                 = delete;
	Output& operator=(const Output&) = delete;
	Output& operator=(Output&&)      = delete;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
