#include <Coral/ShaderLanguage/Variable.hpp>
#include <Coral/ShaderLanguage/Expression.hpp>

using namespace Coral::ShaderLanguage;


Variable::Variable(std::shared_ptr<Expression> source)
	: mSource(source)
{
}


std::shared_ptr<Expression>
Variable::source() const
{
	return mSource;
}


ValueType
Variable::typeId() const
{
	return mSource->resultValueType();
}
