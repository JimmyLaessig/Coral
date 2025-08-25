#ifndef CORAL_SHADERLANGUAGE_CONTROLFLOW_HPP
#define CORAL_SHADERLANGUAGE_CONTROLFLOW_HPP

#include <Coral/ShaderLanguage/Scalar.hpp>

namespace Coral::ShaderLanguage
{

template<typename T>
struct Then;

template<typename T>
struct Else;

struct If
{
public:

	If(Bool condition)
		: mCondition(condition)
	{
	}

	template<typename T>
	Then<T> Then(T then)
	{
		return Then<T>(mCondition, then);
	}

	If(const If& other) = delete;
	If(If&& other) = delete;

	If& operator=(If&& other) = delete;
	If& operator=(const If& other) = delete;

private:

	Bool mCondition;

};


template<typename T>
struct Then
{
public:
	T Else(T value)
	{
		return { std::make_shared<ShaderGraph::ConditionalExpression>(mCondition.source(), mIf.source(), value.source()) };
	}

	Then(const Then& other) = delete;
	Then(Then&& other) = delete;

	Then& operator=(Then&& other) = delete;
	Then& operator=(const Then& other) = delete;

private:

	friend class If;

	Then(Bool condition, T ifCond)
		: mCondition(condition)
		, mIf(ifCond)
	{
	}

	Bool mCondition;
	T mIf;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_CONTROLFLOW_HPP
