#include <Coral/ShaderGraph/CompilerGLSL.hpp>

#include "Visitor.hpp"

#include <cassert>
#include <cmath>
#include <format>
#include <iostream>
#include <map>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>


using namespace Coral::ShaderGraph;

namespace
{

constexpr std::string_view
getTypeShortName(ValueType typeId)
{
	switch (typeId)
	{
		case ValueType::BOOL:    	 return "b";
		case ValueType::INT:       return "i";
		case ValueType::INT2:      return "iv";
		case ValueType::INT3:      return "iv";
		case ValueType::INT4:      return "iv";
		case ValueType::FLOAT:     return "f";
		case ValueType::FLOAT2:    return "v";
		case ValueType::FLOAT3:    return "v";
		case ValueType::FLOAT4:    return "v";
		case ValueType::FLOAT3X3:  return "m";
		case ValueType::FLOAT4X4:  return "m";
		case ValueType::SAMPLER2D: return "s";
	}

	std::unreachable();
}


constexpr std::string_view
toString(ValueType typeId)
{
	switch (typeId)
	{
		case ValueType::BOOL:      return "bool";
		case ValueType::INT:       return "int";
		case ValueType::INT2:      return "ivec2";
		case ValueType::INT3:      return "ivec3";
		case ValueType::INT4:      return "ivec5";
		case ValueType::FLOAT:     return "float";
		case ValueType::FLOAT2:    return "vec2";
		case ValueType::FLOAT3:    return "vec3";
		case ValueType::FLOAT4:    return "vec4";
		case ValueType::FLOAT3X3:  return "mat3";
		case ValueType::FLOAT4X4:  return "mat4";
		case ValueType::SAMPLER2D: return "sampler2D";
	}

	std::unreachable();
}


constexpr Coral::UniformFormat
convert(ValueType typeId)
{
	switch (typeId)
	{
		case ValueType::BOOL:      return Coral::UniformFormat::BOOL;
		case ValueType::INT:       return Coral::UniformFormat::INT32;
		case ValueType::INT2:      return Coral::UniformFormat::VEC2I;
		case ValueType::INT3:      return Coral::UniformFormat::VEC3I;
		case ValueType::INT4:      return Coral::UniformFormat::VEC4I;
		case ValueType::FLOAT:     return Coral::UniformFormat::FLOAT;
		case ValueType::FLOAT2:    return Coral::UniformFormat::VEC2F;
		case ValueType::FLOAT3:    return Coral::UniformFormat::VEC3F;
		case ValueType::FLOAT4:    return Coral::UniformFormat::VEC4F;
		case ValueType::FLOAT3X3:  return Coral::UniformFormat::MAT33F;
		case ValueType::FLOAT4X4:  return Coral::UniformFormat::MAT44F;
		case ValueType::SAMPLER2D: assert(false); return Coral::UniformFormat::INT32;
	}

	std::unreachable();
}


constexpr std::string_view
toString(Coral::UniformFormat type)
{
	switch (type)
	{
		case Coral::UniformFormat::BOOL:   return "bool";
		case Coral::UniformFormat::INT32:	   return "int";
		case Coral::UniformFormat::FLOAT:  return "float";
		case Coral::UniformFormat::VEC2F:  return "vec2";
		case Coral::UniformFormat::VEC3F:  return "vec3";
		case Coral::UniformFormat::VEC4F:  return "vec4";
		case Coral::UniformFormat::VEC2I:  return "ivec2";
		case Coral::UniformFormat::VEC3I:  return "ivec3";
		case Coral::UniformFormat::VEC4I:  return "ivec4";
		case Coral::UniformFormat::MAT33F: return "mat3";
		case Coral::UniformFormat::MAT44F: return "mat4";
	}

	std::unreachable();
}


constexpr std::string_view
toString(Operator op)
{
	switch (op)
	{
		case Operator::MULTIPLY: return "*";
		case Operator::DIVIDE:   return "/";
		case Operator::ADD:	     return "+";
		case Operator::SUBTRACT: return "-";
	}

	std::unreachable();
}


constexpr std::string_view
toString(Swizzle swizzle)
{
	switch (swizzle)
	{
		case Swizzle::X:   return "x";
		case Swizzle::Y:   return "y";
		case Swizzle::Z:   return "z";
		case Swizzle::W:   return "w";
		case Swizzle::XY:  return "xy";
		case Swizzle::XYZ: return "xyz";
	}
	std::unreachable();
}

constexpr auto TAB = "    ";

} // namespace


std::string
CompilerGLSL::format(const Constant<float>& expr)
{
	// Constants are referenced in code directly
	return "";
}


std::string
CompilerGLSL::format(const Constant<int>& expr)
{
	// Constants are referenced in code directly
	return "";
}


std::string
CompilerGLSL::format(const AttributeExpression& expr)
{
	return std::format("{} = {};\n", getRefName(expr), getRefName(expr.inputs().front()));
}


std::string
CompilerGLSL::format(const ParameterExpression& expr)
{
	// Parameters are referenced in code directly
	return "";
}


std::string
CompilerGLSL::format(const OperatorExpression& expr)
{
	assert(expr.inputs().size() == 2);

	auto lhs = expr.inputs()[0];
	auto rhs = expr.inputs()[1];

	return std::format("{} {} = {} {} {};\n",
					   toString(expr.outputValueType()),
					   getRefName(expr),
					   getRefName(lhs),
					   toString(expr.getOperator()),
					   getRefName(rhs));
}


std::string
CompilerGLSL::format(const NativeFunctionExpression& expr)
{
	return std::format("{} {} = {}({});\n",
					   toString(expr.outputValueType()),
					   getRefName(expr),
					   expr.functionName(),
					   buildFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const ConstructorExpression& expr)
{
	return std::format("{} {} = {}({});\n",
					   toString(expr.outputValueType()),
					   getRefName(expr),
					   toString(expr.outputValueType()),
					   buildFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const CastExpression& expr)
{
	assert(expr.inputs().size() == 1);
	return std::format("{} {} = ({}){};\n",
					   toString(expr.outputValueType()),
					   getRefName(expr),
					   toString(expr.outputValueType()),
					   getRefName(expr.inputs().front()));
}


std::string
CompilerGLSL::format(const SwizzleExpression& expr)
{
	// Parameters are referenced in code directly
	return "";
}


std::string
CompilerGLSL::format(const Expression& expr)
{
	return std::visit([this](auto ex) { return format(*ex); }, expr);
}


std::string
CompilerGLSL::getRefName(const ExpressionBase& expr)
{
	auto iter = mNameLookUp.find(&expr);
	if (iter != mNameLookUp.end())
	{
		return iter->second;
	}

	auto varName = std::format("{}{}",getTypeShortName(expr.outputValueType()), mNameLookUp.size());
	mNameLookUp[&expr] = varName;

	return varName;
}


std::string
CompilerGLSL::getRefName(const Constant<float>& expr)
{
	float integral;
	std::string suffix = std::modf(expr.value(), &integral) == 0 ? ".f" : "f";

	return std::format("{}{}", expr.value(), suffix);
}


std::string
CompilerGLSL::getRefName(const Constant<int>& expr)
{
	return std::format("{}", expr.value());
}


std::string
CompilerGLSL::getRefName(const AttributeExpression& expr)
{
	if (expr.inputs().empty())
	{
		return expr.name();
	}

	return std::format("out_{}", expr.name());
}


std::string
CompilerGLSL::getRefName(const ParameterExpression& expr)
{
	return expr.name();
}


std::string
CompilerGLSL::getRefName(const SwizzleExpression& expr)
{
	return std::format("{}.{}", getRefName(expr.inputs().front()), toString(expr.swizzle()));
}


std::string
CompilerGLSL::getRefName(const Expression expr)
{
	return std::visit([this](auto ex) { return getRefName(*ex); }, expr);
}


std::optional<std::pair<uint32_t, uint32_t>>
CompilerGLSL::findUniformBlock(std::string_view parameterName)
{
	for (const auto& [set, bindings] : mDescriptorBindings)
	{
		for (const auto& [binding, definition] : bindings)
		{
			auto res = std::visit(Visitor
			{
				[](auto) { return false; },
				[&](const Coral::UniformBlockDefinition& definition)
				{ 
					auto iter = std::ranges::find_if(definition.members, [&](const auto& member)
													 { return member.name == parameterName; });
					return iter != definition.members.end();
				},
			}, definition.definition);

			if (res)
			{
				return std::pair{set, binding};
			}
		}
	}

	return {};
}


std::string
CompilerGLSL::buildFunctionArgumentList(std::span<const Expression> args)
{
	std::stringstream ss;

	for (auto [i, arg] : std::views::enumerate(args))
	{
		ss << getRefName(arg);

		if (i < args.size() - 1)
		{
			ss << ", ";
		}
	}

	return ss.str();
}


std::string
buildUniformBlockString(uint32_t set, uint32_t binding, const std::string& name, const Coral::UniformBlockDefinition& definition)
{
	std::stringstream ss;

	ss << std::format("layout (std140, set = {}, binding = {}) uniform {}\n", set, binding, name);
	ss << "{" << std::endl;

	for (const auto& [type, name, count, size, paddedSize] : definition.members)
	{
		ss << TAB << toString(type) << " " << name << ";" << std::endl;
	}

	ss << "};" << std::endl;

	return ss.str();
}


std::string
buildUniformCombinedTextureSamplerString(uint32_t set, uint32_t binding, const std::string& name, const Coral::CombinedTextureSamplerDefinition& definition)
{
	return std::format("layout (set = {}, binding = {}) uniform sampler2D {};\n", set, binding, name);
}


bool
CompilerGLSL::createUniformBlockDefinitions()
{
	std::vector<ParameterExpressionPtr> parameters;
	std::unordered_set<ParameterExpressionPtr> inserted;
	for (auto shaderModule : { mShaderProgram->vertexShader(),
							   mShaderProgram->fragmentShader()})
	{
		if (!shaderModule)
		{
			continue;
		}

		for (auto p : shaderModule->parameters())
		{
			if (inserted.insert(p).second)
			{
				parameters.push_back(p);
			}
		}
	}

	Coral::UniformBlockDefinition defaultUniformBlock{};
	
	auto samplers = parameters | std::views::filter([](auto p) { return p->outputValueType() == ValueType::SAMPLER2D; });
	auto uniforms = parameters | std::views::filter([](auto p) { return p->outputValueType() != ValueType::SAMPLER2D; });

	// Find all parameters that are not contained in a UniformBlock override
	for (auto parameter : uniforms)
	{
		if (!findUniformBlock(parameter->name()))
		{
			defaultUniformBlock.members.push_back(Coral::MemberDefinition{ convert(parameter->outputValueType()), 
																		   parameter->name(), 
																		   1 });
		}
	}

	// Add the default uniform block (if required) at the first unused binding
	if (!defaultUniformBlock.members.empty())
	{
		auto& set = mDescriptorBindings[mDefaultDescriptorSet];

		uint32_t binding{ 0 };
		for (; set.find(binding) != set.end(); ++binding) {}

		auto& definition	  = set[binding];
		definition.binding	  = binding;
		definition.definition = defaultUniformBlock;
		definition.name		  = mDefaultUniformBlockName;
		definition.set		  = mDefaultDescriptorSet;
	}

	// Add sampler parameters separately
	for (auto parameter : samplers)
	{
		auto& set = mDescriptorBindings[mDefaultDescriptorSet];

		uint32_t binding{ 0 };
		for (; set.find(binding) != set.end(); ++binding) {}

		auto& definition	  = mDescriptorBindings[mDefaultDescriptorSet][binding];
		definition.binding	  = binding;
		definition.definition = Coral::CombinedTextureSamplerDefinition{};
		definition.name		  = parameter->name();
		definition.set		  = mDefaultDescriptorSet;
		binding++;
	}
	return true;
}


bool
CompilerGLSL::createAttributeLocationDefinitions()
{
	auto shaderModules = std::array{ mShaderProgram->vertexShader(), mShaderProgram->fragmentShader() }
		| std::views::filter([&](auto shaderModule) { return shaderModule != nullptr; })
		| std::ranges::to<std::vector<const ShaderGraph::ShaderModule*>>();

	for (auto[i, shaderModule]: std::views::enumerate(shaderModules))
	{
		auto& bindings = mShaderStageAttributeBindingsLookUp[shaderModule];
		// Create the input locations
		if (i == 0)
		{
			for (auto [location, attribute] : std::views::enumerate(shaderModule->inputs()))
			{
				bindings.inputAttributes[attribute] = location;
			}
		}
		else
		{
			auto prevShaderModule = shaderModules[i - 1];
			auto& prevBindings = mShaderStageAttributeBindingsLookUp[prevShaderModule];
			for (auto attribute : shaderModule->inputs())
			{
				auto linkedAttribute = std::find_if(prevBindings.outputAttributes.begin(), prevBindings.outputAttributes.end(),
					[&](const auto& pair) { return pair.first->name() == attribute->name() && pair.first->outputValueType() == attribute->outputValueType(); });

				if (linkedAttribute == prevBindings.outputAttributes.end())
				{
					assert(false);
				}
				
				bindings.inputAttributes[attribute] = prevBindings.outputAttributes[linkedAttribute->first];
			}
		}

		// Create the output locations
		for (auto [location, parameter] : std::views::enumerate(shaderModule->outputs()))
		{
			bindings.outputAttributes[parameter] = location;
		}
	}

	return true;
}


std::string
CompilerGLSL::buildUniformBlocksString()
{
	std::stringstream ss;

	for (const auto& [set, bindings] : mDescriptorBindings)
	{
		for (const auto& [binding, definition] : bindings)
		{
			std::visit(Visitor
			{
				[&](const Coral::UniformBlockDefinition& uniformBlock)
				{
					ss << buildUniformBlockString(definition.set, definition.binding, definition.name, uniformBlock) << std::endl;
				},
				[&](auto)
				{
				},
				[&](const Coral::CombinedTextureSamplerDefinition& sampler)
				{
					ss << buildUniformCombinedTextureSamplerString(definition.set, definition.binding, definition.name, sampler) << std::endl;
				}
			}, definition.definition);
		}
	}

	

	return ss.str();
}


std::string
CompilerGLSL::buildInputAttributeDefinitionsString(const ShaderModule& shaderModule)
{
	std::map<uint32_t, AttributeExpressionPtr> attributesSorted;
	for (const auto& [attr, location] : mShaderStageAttributeBindingsLookUp[&shaderModule].inputAttributes)
	{
		attributesSorted[location] = attr;
	}

	std::stringstream ss;
	for (auto [location, attr] : attributesSorted)
	{
		ss << std::format("layout (location = {}) in {} {};\n",
						  location,
						  toString(attr->outputValueType()),
						  getRefName(*attr));
	}

	return ss.str();
}


std::string
CompilerGLSL::buildOutputAttributeDefinitionsString(const ShaderModule& shaderModule)
{
	std::map<uint32_t, AttributeExpressionPtr> attributesSorted;
	for (const auto& [attr, location] : mShaderStageAttributeBindingsLookUp[&shaderModule].outputAttributes)
	{
		attributesSorted[location] = attr;
	}

	std::stringstream ss;
	for (auto [location, attr] : attributesSorted)
	{
		ss << std::format("layout (location = {}) out {} {};\n",
						  location,
						  toString(attr->outputValueType()),
						  getRefName(*attr));
	}

	return ss.str();
}


std::string
CompilerGLSL::buildMainFunctionString(const ShaderModule& shaderModule)
{
	auto expressionList = shaderModule.buildExpressionList();

	std::unordered_set<Expression> visited;

	std::stringstream ss;

	ss << "void main()" << std::endl;
	ss << "{" << std::endl;

	for (const auto& expr : expressionList)
	{
		if (visited.contains(expr))
		{
			continue;
		}

		std::visit(Visitor{ 
			[&](auto ex) { ss << TAB << format(expr); },
			[&](std::shared_ptr<AttributeExpression> attr)
			{
				if (!attr->inputs().empty())
				{
					ss << TAB << format(*attr);
				}
			},
			[](std::shared_ptr<SwizzleExpression>) {},
			[](std::shared_ptr<ParameterExpression>) {},
			[](std::shared_ptr<Constant<float>>) {},
			[](std::shared_ptr<Constant<int>>) {},
		}, expr);

		visited.insert(expr);
	}

	for (auto output : shaderModule.outputs())
	{
		if (output->name() == DefaultSemantics::Position && shaderModule.shaderStage() == ShaderStage::VERTEX)
		{
			ss << TAB << std::format("gl_Position = {};", getRefName(output->inputs().front())) << std::endl;
		}
		if (output->name() == DefaultSemantics::Depth && shaderModule.shaderStage() == ShaderStage::FRAGMENT)
		{
			ss << TAB << std::format("gl_FragDepth = {};", getRefName(output->inputs().front())) << std::endl;
		}
	}

	ss << "}";

	return ss.str();
}


Compiler&
CompilerGLSL::setShaderProgram(const Program& shaderProgram)
{
	mShaderProgram = &shaderProgram;
	return *this;
}


Compiler&
CompilerGLSL::addUniformBlockOverride(uint32_t set, uint32_t binding, std::string_view name, const Coral::UniformBlockDefinition& uniformBlock)
{
	auto& definition      = mDescriptorBindings[set][binding];
	definition.binding    = binding;
	definition.byteSize   = 0;
	definition.name		  = name;
	definition.set		  = set;
	definition.definition = uniformBlock;
	return *this;
}


Compiler&
CompilerGLSL::setDefaultUniformBlockName(std::string_view name)
{
	mDefaultUniformBlockName = name;
	return *this;
}


Compiler&
CompilerGLSL::setDefaultDescriptorSet(uint32_t set)
{
	mDefaultDescriptorSet = set;
	return *this;
}

std::optional<CompilerResult>
CompilerGLSL::compile()
{
	if (!mShaderProgram)
	{
		return {};
	}

	if (!createUniformBlockDefinitions())
	{
		return {};
	}

	if (!createAttributeLocationDefinitions())
	{
		return {};
	}
	CompilerResult result;

	for (auto [shaderModule, source] : { std::pair{ mShaderProgram->vertexShader(), &result.vertexShader},
									     std::pair{ mShaderProgram->fragmentShader(), &result.fragmentShader } })
	{
		if (shaderModule)
		{
			auto inputAttributes  = buildInputAttributeDefinitionsString(*shaderModule);
			auto outputAttributes = buildOutputAttributeDefinitionsString(*shaderModule);
			auto uniforms	      = buildUniformBlocksString();
			auto mainFunc	      = buildMainFunctionString(*shaderModule);

			std::stringstream ss;
			ss << "#version 420" << std::endl;
			ss << inputAttributes << std::endl;
			ss << outputAttributes << std::endl;
			ss << uniforms << std::endl;
			ss << mainFunc << std::endl;

			*source = ss.str();
		}
	}



	return result;
}
