#ifndef CORAL_SHADERLANGUAGE_SHADERGRAPH_HPP
#define CORAL_SHADERLANGUAGE_SHADERGRAPH_HPP

#include <Coral/ShaderLanguage/Expressions.hpp>
#include <memory>
#include <vector>
#include <cassert>

namespace Coral::ShaderLanguage
{

// ============================================================
//  FunctionTraits — handles function pointers
// ============================================================

template <typename T>
struct FunctionTraits;

// function pointer
template <typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)>
{
	using ReturnType = R;
	using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;
};


class ShaderGraph
{
public:

	template<typename Fun>
	void SetShaderFunction(Fun f)
	{
		InvokeShaderFunction(f);
	}

	std::vector<const InputAttributeExpression*> Inputs() const;

	std::vector<const OutputAttributeExpression*> Outputs() const;

	std::vector<const UniformBufferExpression*> UniformBuffers() const;

	std::vector<const Expression*> ExpressionList() const;

	template<typename T, typename ...Args>
	static std::shared_ptr<T> PushExpression(Args&&... args)
	{
		auto expression = std::make_shared<T>(std::forward<Args>(args)...);
		if (sCurrentShaderModule)
		{
			sCurrentShaderModule->mInstructions.push_back(expression);
		}
		else
		{
			assert(false);
		}

		return expression;
	}

	template<typename T, typename ...Args>
	static std::shared_ptr<Expression> ReplaceExpression(std::shared_ptr<Expression> old, Args&&... args)
	{
		auto expression = std::make_shared<T>(std::forward<Args>(args)...);

		ReplaceExpressionImpl(old, expression);

		return expression;
	}

private:

	static void ReplaceExpressionImpl(std::shared_ptr<Expression> old, std::shared_ptr<Expression> newExpression);

	template<typename Func>
	void InvokeShaderFunction(Func func)
	{
		using ArgsTuple = typename FunctionTraits<Func>::ArgsTuple;

		sCurrentShaderModule = this;

		{
			auto result = std::apply(func, ArgsTuple{});
		}

		sCurrentShaderModule = nullptr;
	}

	static inline ShaderGraph* sCurrentShaderModule{ nullptr };

	std::vector<ExpressionPtr> mInstructions;

}; // class ShaderModule

} // namespace Coral::ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SHADERGRAPH_HPP
