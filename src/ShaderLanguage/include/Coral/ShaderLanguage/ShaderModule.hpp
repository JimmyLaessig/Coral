#ifndef CORAL_SHADERLANGUAGE_SHADERMODULE_HPP
#define CORAL_SHADERLANGUAGE_SHADERMODULE_HPP

#include <Coral/ShaderLanguage/Expression.hpp>
#include <Coral/ShaderLanguage/Value.hpp>
#include <cassert>
#include <memory>
#include <vector>

#include <print>

namespace Coral::ShaderLanguage
{

class ShaderModule
{
public:

	ShaderModule() = default;

	virtual ~ShaderModule() = default;

	void buildInstructionList()
	{
		sCurrentShaderModule = this;
		main();
		sCurrentShaderModule = nullptr;
	}

	static ShaderModule* current()
	{
		assert(sCurrentShaderModule != nullptr);
		return sCurrentShaderModule;
	}

	void addExpression(std::shared_ptr<Expression> ex);

	std::vector<const InputAttributeExpression*> inputs() const;

	std::vector<const OutputAttributeExpression*> outputs() const;

	std::vector<const UniformBufferExpression*> parameters() const;

	std::vector<const Expression*> expressionList() const;

	template<typename T, typename ...Args>
	std::shared_ptr<Expression> addExpression(Args&&... args)
	{
		auto expression = std::make_shared<T>(std::forward<Args>(args)...);
		mInstructions.push_back(expression);
		return expression;
	}

protected:

	virtual void main() = 0;

private:

	static inline ShaderModule* sCurrentShaderModule{ nullptr };

	std::vector<std::shared_ptr<Expression>> mInstructions;

	std::unordered_map<Expression*, std::string> mNameLookUp;

	size_t mNameCounter{ 0 };
}; // class ShaderModule


template<typename T, typename ...Args>
std::shared_ptr<Expression> pushExpression(Args&&... args)
{
	return ShaderModule::current()->addExpression<T>(std::forward<Args>(args)...);
}

} // namespace Coral::ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SHADERMODULE_HPP
