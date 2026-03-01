#ifndef CORAL_SHADERLANGUAGE_UNIFORMBUFFER_HPP
#define CORAL_SHADERLANGUAGE_UNIFORMBUFFER_HPP

#include <Coral/ShaderLanguage/Binding.hpp>
#include <Coral/ShaderLanguage/Expressions.hpp>
#include <Coral/ShaderLanguage/ShaderGraph.hpp>


namespace Coral::ShaderLanguage
{

class UniformBufferBase
{
public:

	template<StringLiteral Name>
	void RegisterMember(Value& member)
	{
		auto expression = ShaderGraph::ReplaceExpression<UniformExpression>(member.source(), member.typeId(), Name, mBufferExpression);
		member.setSource(expression);
	}

protected:
	
	UniformBufferBase(uint32_t location, std::string_view name)
		: mBufferExpression(ShaderGraph::PushExpression<UniformBufferExpression>(location, name))
	{
	}

private:

	std::shared_ptr<UniformBufferExpression> mBufferExpression;
};


template<typename T, StringLiteral Name, Location L>
class UniformBuffer : protected UniformBufferBase
{
public:

	UniformBuffer()
		: UniformBufferBase(static_cast<uint32_t>(L), Name)
	{
		DefineType(mValue, static_cast<UniformBufferBase&>(*this));
	}

	const T* operator->() const
	{
		return &mValue;
	}

	const T& operator*() const
	{
		return mValue;
	}

	operator T() const
	{
		return mValue;
	}

private:

	T mValue;
};

} // namespace Coral::ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_UNIFORMBUFFER_HPP
