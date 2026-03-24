#include <Coral/ShaderLanguage/Value.hpp>
#include <Coral/ShaderLanguage/Node.hpp>

#include <Coral/ShaderLanguage/UniformBuffer.hpp>

using namespace Coral::ShaderLanguage;

Value::Value(std::shared_ptr<Node> source)
	: mNode(source)
{
}


std::shared_ptr<Node>
Value::node() const &
{
	return mNode;
}


std::shared_ptr<Node>
Value::node() &&
{
	// Mark the source of this expression to be inlined if this is an r-value reference
	mNode->setInlineIfPossible();
	return mNode;
}


void
Value::setNode(std::shared_ptr<Node> source)
{
	mNode = source;
}
