#ifndef CORAL_SHADERLANGUAGE_PARAMETER_HPP
#define CORAL_SHADERLANGUAGE_PARAMETER_HPP

namespace Coral::ShaderLanguage
{

template<typename T>
struct Parameter final : public T
{
	Parameter(std::string_view name)
		: T(std::make_shared<ShaderGraph::ParameterExpression>(T::toShaderTypeId(), name))
	{
	}

	Parameter(const Parameter&) = delete;
	Parameter(Parameter&&) = delete;
	Parameter& operator=(const Parameter&) = delete;
	Parameter& operator=(Parameter&&) = delete;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_PARAMETER_HPP
