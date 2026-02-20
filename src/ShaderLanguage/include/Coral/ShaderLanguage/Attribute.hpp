#ifndef CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
#define CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP

#include <Coral/ShaderLanguage/Expression.hpp>
#include <Coral/ShaderLanguage/Value.hpp>
#include <concepts>

namespace Coral::ShaderLanguage
{
class ShaderModule;


class NonCopyable
{
public:
	virtual ~NonCopyable() = default;
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

class NonMoveable
{
public:
	virtual ~NonMoveable() = default;
	NonMoveable() = default;
	NonMoveable(NonMoveable&&) = delete;
	NonMoveable& operator=(NonMoveable&&) = delete;
};


class Container : private NonCopyable, private NonMoveable
{
public:

	static Container* Current()
	{
		return !sContainerStack.empty() ? sContainerStack.back() : nullptr;
	}

protected:

	uint8_t BeginScope()
	{
		sContainerStack.push_back(this);
		std::println("BeginScope");
		return 0;
	}

	uint8_t EndScope()
	{
		assert(sContainerStack.back() == this);
		sContainerStack.pop_back();
		std::println("EndScope");
		return 0;
	}

private:

	static thread_local std::vector<Container*> sContainerStack;
};


thread_local std::vector<Container*> Container::sContainerStack;


enum Location : uint8_t {};

enum Qualifier
{
	IN,
	OUT,
};


template<typename T, Location L>
class UniformBuffer : private Container
{
public:

	UniformBuffer(std::string_view name)
		: mName(name)
	{
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

	std::string mName;
	uint8_t mBegin{ BeginScope() };
	T mValue{};
	uint8_t mEnd{ EndScope() };
};


template<typename T, Location L, Qualifier Q, StringLiteral Name>
class Attribute : private Container
{
public:

	Attribute() requires (Q == Qualifier::IN)
		: mValue(pushExpression<InputAttributeExpression>(T::toValueType(), Name.value))
	{
	}

	Attribute() requires (Q == Qualifier::OUT)
		: mValue(pushExpression<OutputAttributeExpression>(T::toValueType(), Name.value))
	{
	}

	const T* operator->() const requires (Q == Qualifier::IN)
	{
		return &mValue;
	}

	const T& operator*() const  requires (Q == Qualifier::IN)
	{
		return mValue;
	}

	operator T() const  requires (Q == Qualifier::IN)
	{
		return mValue;
	}

	Attribute& operator=(const T& t) requires (Q == Qualifier::OUT)
	{
		// TODO fix this
		//mValue = T(pushExpression<OperatorExpression>(mValue.toValueType(), mValue.source(), Operator::ASSIGNMENT, other.source());
		return *this;
	}

	Attribute& operator=(T&& t) requires (Q == Qualifier::OUT)
	{
		// TODO fix this
		//mValue = T(pushExpression<OperatorExpression>(mValue.toValueType(), mValuesource(), Operator::ASSIGNMENT, std::forward<T&&>(other).source());
		return *this;
	}

private:
	T mValue{};
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
