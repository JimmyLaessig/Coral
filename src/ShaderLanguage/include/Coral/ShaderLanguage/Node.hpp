#ifndef CORAL_SHADERLANGUAGE_NODE_HPP
#define CORAL_SHADERLANGUAGE_NODE_HPP

#include <Coral/ShaderLanguage/Expressions.hpp>

#include <Coral/ShaderLanguage/Visitor.hpp>

#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <memory>
#include <vector>
#include <ranges>


namespace Coral::ShaderLanguage
{

/// Class representing an node in the shader graph
class Node : public std::enable_shared_from_this<Node>
{
public:

	template<typename ExpressionType>
	Node(ExpressionType&& expression, const std::vector<NodePtr>& inputs)
		: mExpression(std::forward<ExpressionType>(expression))
		, mInputs(inputs)
	{
	}

	template<typename ExpressionType>
	static NodePtr create(ExpressionType&& expression)
	{
		auto node = std::make_shared<Node>(std::forward<ExpressionType>(expression), std::vector<NodePtr>{});

		std::visit(Visitor{ [&](auto& ex) { ex.setNode(node); } }, node->mExpression);

		if (auto shaderGraph = ShaderGraph::current())
		{
			shaderGraph->addNode(node);
		}

		return node;
	}

	template<typename ExpressionType, typename ...Nodes>
	static NodePtr create(ExpressionType&& expression, Nodes&&... inputs)
	{
		auto node = std::make_shared<Node>(std::forward<ExpressionType>(expression), std::vector<NodePtr>{ std::forward<Nodes>(inputs)... });

		std::visit(Visitor{ [&](auto& ex) { ex.setNode(node); } }, node->mExpression);

		if (auto shaderGraph = ShaderGraph::current())
		{
			shaderGraph->addNode(node);
		}


		return node;
	}

	// Get the type of the output value of this expression
	ValueType valueType() const
	{
		return std::visit(Visitor{ [](const auto& ex) { return ex.valueType(); } }, mExpression);
	}

	const Expression& expression() const
	{
		return mExpression;
	}

	template<typename ExpressionType>
	void setExpression(ExpressionType&& expression)
	{
		mExpression = std::forward<ExpressionType>(expression);
		auto self = shared_from_this();
		std::visit(Visitor{ [self](auto& ex) { ex.setNode(self); } }, mExpression);
	}

	// Get the list of inputs this expression
	std::vector<std::shared_ptr<const Node>> inputs() const
	{
		return mInputs | std::ranges::to<std::vector<std::shared_ptr<const Node>>>();
	}

	bool inlineIfPossible() const
	{
		return mInline;
	}

	void setInlineIfPossible()
	{
		mInline = true;
	}

private:

	//static void init(std::shared_ptr<Expression> expression);

	//friend class ShaderGraph;
	Expression mExpression;

	std::vector<NodePtr> mInputs;

	bool mInline{ false };

}; // class Node

} // namespace Coral::ShaderLanguage

#endif // !CORAL_SHADERLANGUAGE_NODE_HPP
