#include <Coral/ShaderLanguage/Value.hpp>
#include <Coral/ShaderLanguage/Expression.hpp>

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
	mSource->setInlineIfPossible();
	return mSource;
}


void
Value::setSource(std::shared_ptr<Expression> ex)
{
	mSource = ex;
}

ValueType
Value::typeId() const
{
	return mSource->valueTypeId();
}



