#include <Coral/ShaderLanguage/Value.hpp>
#include <Coral/ShaderLanguage/Expression.hpp>

#include <Coral/ShaderLanguage/UniformBuffer.hpp>

using namespace Coral::ShaderLanguage;

Value::Value(std::shared_ptr<Expression> source)
	: mSource(source)
{
}

std::shared_ptr<Expression>
Value::source() const &
{
	return mSource;
}


std::shared_ptr<Expression>
Value::source() &&
{
	// Mark the source of this expression to be inlined if this is an r-value reference
	mSource->SetInlineIfPossible();
	return mSource;
}


ValueType
Value::typeId() const
{
	return mSource->GetValueType();
}


void
Value::setSource(std::shared_ptr<Expression> source)
{
	mSource = source;
}
