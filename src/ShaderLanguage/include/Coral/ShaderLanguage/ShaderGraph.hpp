#ifndef CORAL_SHADERLANGUAGE_SHADERGRAPH_HPP
#define CORAL_SHADERLANGUAGE_SHADERGRAPH_HPP

#include <Coral/ShaderLanguage/Expressions.hpp>
#include <memory>
#include <vector>
#include <cassert>

namespace Coral::ShaderLanguage
{

class Node;

// ============================================================
//  FunctionTraits — handles function pointers
// ============================================================

template <typename T>
struct FunctionTraits;

template <typename C, typename R, typename... Args>
struct FunctionTraits<R(C::*)(Args...)>
{
	using ClassType  = C;
	using ReturnType = R;
	using ArgsTuple  = std::tuple<std::remove_cvref_t<Args>...>;
};


class ShaderGraph
{
public:

	template<typename T>
	ShaderGraph(T&& shader)
	{
		auto main = &T::main;

		using Args = FunctionTraits<decltype(main)>::ArgsTuple;

		sCurrentShaderModule = this;
		{
			Args args{};

			auto result = std::apply(
				[&](auto&&... unpacked) {
					return (shader.*main)(std::forward<decltype(unpacked)>(unpacked)...);
				},
				args
			);
		}

		sCurrentShaderModule = nullptr;
	}

	std::vector<const InputAttributeExpression*> inputs() const;

	std::vector<const OutputAttributeExpression*> outputs() const;

	std::vector<const UniformExpression*> uniforms() const;

	std::vector<std::shared_ptr<const Node>> expressions() const;

	void addNode(std::shared_ptr<Node> node);

	static ShaderGraph* current() { return sCurrentShaderModule; }

private:

	static inline ShaderGraph* sCurrentShaderModule{ nullptr };

	std::vector<NodePtr> mNodes;

}; // class ShaderModule

} // namespace Coral::ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SHADERGRAPH_HPP
