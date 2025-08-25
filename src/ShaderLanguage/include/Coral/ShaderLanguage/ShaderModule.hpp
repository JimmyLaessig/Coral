#ifndef CORAL_SHADERLANGUAGE_SHADERMODULE_HPP
#define CORAL_SHADERLANGUAGE_SHADERMODULE_HPP

#include <Coral/ShaderLanguage/Attribute.hpp>

#include <memory>
#include <vector>

namespace Coral::ShaderLanguage
{

class ShaderModule
{
public:

	/// Register a new shader output attribute
	template<typename T>
	void registerOutputAttribute(const Output<T>& attribute)
	{
		registerOutputAttribute(std::static_pointer_cast<ShaderGraph::OutputAttributeExpression>(attribute.source()));
	}

	/// Get the input attributes of the shader module
	std::vector<const ShaderGraph::InputAttributeExpression*> inputs() const;

	/// Get the parameters used by this shader module
	std::vector<const ShaderGraph::ParameterExpression*> parameters() const;

	/// Get the output attributes of the shader module
	std::vector<const ShaderGraph::OutputAttributeExpression*> outputs() const;

	/// Build a flattened list of all shader expression
	std::vector<const ShaderGraph::Expression*> buildExpressionList() const;

private:

	void registerOutputAttribute(std::shared_ptr<ShaderGraph::OutputAttributeExpression> attribute);

	std::vector<std::shared_ptr<ShaderGraph::OutputAttributeExpression>> mOutputs;
};

} // namespace Coral::ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SHADERMODULE_HPP
