#ifndef CORAL_SHADERLANGUAGE_SHADERMODULE_HPP
#define CORAL_SHADERLANGUAGE_SHADERMODULE_HPP

#include <Coral/ShaderLanguage/Expression.hpp>
#include <Coral/ShaderLanguage/Variable.hpp>
#include <cassert>
#include <memory>
#include <vector>

namespace Coral::ShaderLanguage
{
template<typename T>
class ReadOnly : public T
{
public:
	using T::T;

	ReadOnly(const T&) = delete;
	ReadOnly(T&&) = delete;
	ReadOnly& operator=(const T&) = delete;
	ReadOnly& operator=(T&&) = delete;
};


class ShaderModule
{
public:

	ShaderModule()
	{

	}

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

	std::vector<const ParameterExpression*> parameters() const;

	std::vector<const Expression*> expressionList() const;

	template<typename T, typename ...Args>
	std::shared_ptr<Expression>addExpression(Args&&... args)
	{
		auto expression = std::make_shared<T>(std::forward<Args>(args)...);
		mInstructions.push_back(expression);
		return expression;
	}

protected:

	virtual ~ShaderModule() = default;

	virtual void main() = 0;

private:

	static inline ShaderModule* sCurrentShaderModule{ nullptr };

	std::vector<std::shared_ptr<Expression>> mInstructions;

	std::unordered_map<Expression*, std::string> mNameLookUp;

	size_t mNameCounter{ 0 };
}; // class ShaderModule

} // namespace Coral::ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SHADERMODULE_HPP
