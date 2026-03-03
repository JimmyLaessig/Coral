#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <Coral/ShaderLanguage/Visitor.hpp>

#include <Coral/ShaderLanguage/Node.hpp>


#include <algorithm>
#include <cassert>
#include <ranges>
#include <set>
#include <unordered_set>

using namespace Coral::ShaderLanguage;


std::vector<const InputAttributeExpression*>
ShaderGraph::inputs() const
{
	std::vector<const InputAttributeExpression*> result;
	std::set<const InputAttributeExpression*> visited;
	for (auto node : mNodes)
	{
		if (auto attr = std::get_if<InputAttributeExpression>(&node->expression()))
		{
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const UniformExpression*>
ShaderGraph::uniforms() const
{
	std::vector<const UniformExpression*> result;
	std::set<const UniformExpression*> visited;
	for (auto node : mNodes)
	{
		if (auto uniform = std::get_if<UniformExpression>(&node->expression()))
		{
			if (visited.insert(uniform).second)
			{
				result.push_back(uniform);
			}
		}
	}

	return result;
}


std::vector<const OutputAttributeExpression*>
ShaderGraph::outputs() const
{
	std::vector<const OutputAttributeExpression*> result;
	std::set<const OutputAttributeExpression*> visited;

	for (auto node : mNodes)
	{
		if (auto attr = std::get_if<OutputAttributeExpression>(&node->expression()))
		{
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<std::shared_ptr<const Node>>
ShaderGraph::expressions() const
{
	return mNodes
		| std::views::as_const
		| std::ranges::to<std::vector<std::shared_ptr<const Node>>>();
}


void
ShaderGraph::addNode(std::shared_ptr<Node> node)
{
	mNodes.push_back(node);
}
