#ifndef CORAL_SHADERLANGUAGE_UNIFORMBUFFER_HPP
#define CORAL_SHADERLANGUAGE_UNIFORMBUFFER_HPP

#include <Coral/ShaderLanguage/Binding.hpp>
#include <Coral/ShaderLanguage/Expressions.hpp>
#include <Coral/ShaderLanguage/ShaderGraph.hpp>
#include <Coral/ShaderLanguage/Node.hpp>

namespace Coral::ShaderLanguage
{

class UniformBufferBase
{
public:

	template<StringLiteral Name, typename T>
	void RegisterMember(T& member)
	{
		member.node()->setExpression(UniformExpression(T::ValueType, Name, mBufferInfo));
	}

protected:

	UniformBufferBase(uint32_t location, std::string_view name)
		: mBufferInfo(std::make_shared<UniformBufferInfo>(location, name))
	{
	}

private:

	std::shared_ptr<UniformBufferInfo> mBufferInfo;
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
