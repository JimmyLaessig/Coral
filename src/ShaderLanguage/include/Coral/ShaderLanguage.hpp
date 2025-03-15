#ifndef CORAL_SHADERLANGUAGE_HPP
#define CORAL_SHADERLANGUAGE_HPP

#include <Coral/ShaderLanguage/Vector.hpp>
#include <Coral/ShaderLanguage/Matrix.hpp>
#include <Coral/ShaderLanguage/Scalar.hpp>
#include <Coral/ShaderLanguage/Functions.hpp>
#include <Coral/ShaderLanguage/Sampler.hpp>

namespace Coral::ShaderLanguage
{

template<typename T>
inline T Attribute(std::string_view name)
{
	return { ShaderGraph::Node::createAttribute(T::toShaderTypeId(), name)};
}


template<typename T>
inline T Parameter(std::string_view name)
{
	return { ShaderGraph::Node::createParameter(T::toShaderTypeId(), name) };
}


template<typename T>
inline T Conditional(Bool condition, T ifBranch, T elseBranch)
{
	return { ShaderGraph::Node::createCondition(condition.source(), ifBranch.source(), elseBranch.source())};
}


using DefaultSemantics = Coral::ShaderGraph::DefaultSemantics;


template<typename T>
struct Then
{
public:
	T Else(T value)
	{
		return { ShaderGraph::Node::createCondition(mCondition.source(), mIf.source(), value.source()) };
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


struct If
{
public:
	
	If(Bool condition)
		: mCondition(condition)
	{
	}

	template<typename T>
	Coral::ShaderLanguage::Then<T> Then(T then)
	{
		return Coral::ShaderLanguage::Then<T>(mCondition, then);
	}

	If(const If& other) = delete;
	If(If&& other) = delete;

	If& operator=(If&& other) = delete;
	If& operator=(const If& other) = delete;
	
private:

	Bool mCondition;

};


class Shader
{
public:

	template<typename T>
	void addOutput(std::string_view name, T value)
	{
		mShaderGraph.addOutput(name, value.source());
	}

	template<typename T>
	void addOutput(Coral::ShaderGraph::DefaultSemantics semantics, T value)
	{
		mShaderGraph.addOutput(semantics, value.source());
	}

	ShaderGraph::Shader& shaderGraph()
	{
		return mShaderGraph;
	}

private:

	ShaderGraph::Shader mShaderGraph;
};

} // namespace ShaderLanguage 

namespace csl = Coral::ShaderLanguage;

#endif // !CORAL_SHADERLANGUAGE_HPP