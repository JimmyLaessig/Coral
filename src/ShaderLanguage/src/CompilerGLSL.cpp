#include <Coral/ShaderLanguage/CompilerGLSL.hpp>

#include "Visitor.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <format>
#include <iostream>
#include <map>
#include <ranges>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>


using namespace Coral::ShaderLanguage;
using namespace Coral::ShaderLanguage::ShaderGraph;

namespace
{

constexpr std::string_view
getTypeShortName(ValueType typeId)
{
	switch (typeId)
	{
		case ValueType::BOOL:      return "b";
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
		case Coral::UniformFormat::INT32:  return "int";
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
		case Operator::MULTIPLY:         return "*";
		case Operator::DIVIDE:           return "/";
		case Operator::ADD:	             return "+";
		case Operator::SUBTRACT:         return "-";
		case Operator::EQUAL:            return "==";
		case Operator::NOT_EQUAL:        return "!=";
		case Operator::GREATER:		     return ">";
		case Operator::LESS:			 return "<";
		case Operator::GREATER_OR_EQUAL: return ">=";
		case Operator::LESS_OR_EQUAL:    return "<=";
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


constexpr std::string_view
toString(DefaultAttribute attribute)
{
	switch (attribute)
	{
		case DefaultAttribute::POSITION: return "gl_Position";
		case DefaultAttribute::DEPTH:    return "gl_FragDepth";
	}
	std::unreachable();
}


constexpr auto TAB = "    ";


template<typename Visitor>
auto visit(const ShaderGraph::Expression& expr, Visitor visitor)
{
	switch (expr.typeId())
	{
		case Expression::TypeId::CONSTANT_FLOAT:   return visitor(static_cast<const ConstantExpression<float>&>(expr));
		case Expression::TypeId::CONSTANT_INT:     return visitor(static_cast<const ConstantExpression<int>&>(expr));
		case Expression::TypeId::CONSTANT_BOOL:    return visitor(static_cast<const ConstantExpression<bool>&>(expr));
		case Expression::TypeId::INPUT_ATTRIBUTE:  return visitor(static_cast<const InputAttributeExpression&>(expr));
		case Expression::TypeId::OUTPUT_ATTRIBUTE: return visitor(static_cast<const OutputAttributeExpression&>(expr));
		case Expression::TypeId::PARAMETER:        return visitor(static_cast<const ParameterExpression&>(expr));
		case Expression::TypeId::NATIVE_FUNCTION:  return visitor(static_cast<const NativeFunctionExpression&>(expr));
		case Expression::TypeId::CONSTRUCTOR:      return visitor(static_cast<const ConstructorExpression&>(expr));
		case Expression::TypeId::CAST:             return visitor(static_cast<const CastExpression&>(expr));
		case Expression::TypeId::SWIZZLE:          return visitor(static_cast<const SwizzleExpression&>(expr));
		case Expression::TypeId::CONDITIONAL:      return visitor(static_cast<const ConditionalExpression&>(expr));
		case Expression::TypeId::OPERATOR:         return visitor(static_cast<const OperatorExpression&>(expr));
	}

	std::unreachable();
}

} // namespace


std::string
CompilerGLSL::format(const ConstantExpression<float>& expr)
{
	float integral;
	std::string suffix = std::modf(expr.value(), &integral) == 0 ? ".f" : "f";
	return std::format("{}{}", expr.value(), suffix);
}


std::string
CompilerGLSL::format(const ConstantExpression<int>& expr)
{
	return std::to_string(expr.value());
}


std::string
CompilerGLSL::format(const ConstantExpression<bool>& expr)
{
	return expr.value() ? "true" : "false";
}


std::string
CompilerGLSL::format(const InputAttributeExpression& expr)
{
	return expr.attribute();
}


std::string
CompilerGLSL::format(const OutputAttributeExpression& expr)
{
	return std::visit(Visitor{
		[](DefaultAttribute a) { return std::string(toString(a)); },
		[](const std::string& attribute) { return std::format("out_{}", attribute); },
		}, expr.attribute());
}


std::string
CompilerGLSL::format(const ParameterExpression& expr)
{
	return expr.name();
}


std::string
CompilerGLSL::format(const OperatorExpression& expr)
{
	auto inputs = expr.inputs();

	return std::format("({} {} {})",
					   resolve(*inputs[0]),
					   toString(expr.getOperator()),
		               resolve(*inputs[1]));
}


std::string
CompilerGLSL::format(const NativeFunctionExpression& expr)
{
	return std::format("{}({})",
					   expr.functionName(),
					   formatFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const ConstructorExpression& expr)
{
	return std::format("{}({})",
					   toString(expr.outputValueType()),
					   formatFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const CastExpression& expr)
{
	auto inputs = expr.inputs();
	return std::format("({}){}",
					   toString(expr.outputValueType()),
					   resolve(*inputs.front()));
}


std::string
CompilerGLSL::format(const SwizzleExpression& expr)
{
	auto inputs = expr.inputs();
	return std::format("{}.{}", 
		               resolve(*inputs.front()),
	                   toString(expr.swizzle()));
}


std::string
CompilerGLSL::format(const ConditionalExpression& expr)
{
	auto inputs = expr.inputs();
	return std::format("({} ? {} : {})",
					   resolve(*inputs[0]),
					   resolve(*inputs[1]),
					   resolve(*inputs[2]));
}


std::string
CompilerGLSL::format(const Expression* expr)
{
	return visit(*expr, Visitor{ [this](const auto& e) { return format(e); } });
}


std::string
CompilerGLSL::resolve(const ShaderGraph::Expression& expr)
{
	auto iter = mNameLookUp.find(&expr);
	if (iter != mNameLookUp.end())
	{
		return iter->second;
	}

	return format(&expr);
}


std::optional<uint32_t>
CompilerGLSL::findUniformBinding(std::string_view parameterName)
{
	for (const auto& [binding, descriptor] : mDescriptorBindings)
	{
		auto res = std::visit(Visitor
		{
			[&](const Coral::CombinedTextureSamplerDefinition&) { return descriptor.name == parameterName; },
			[&](const Coral::SamplerDefinition&)			    { return descriptor.name == parameterName; },
			[&](const Coral::TextureDefinition&)			    { return descriptor.name == parameterName; },
			[&](const Coral::UniformBlockDefinition& definition)
			{ 
				auto iter = std::ranges::find_if(definition.members, [&](const auto& member)
													{ return member.name == parameterName; });
				return iter != definition.members.end();
			},
		}, descriptor.definition);

		if (res)
		{
			return binding;
		}
	}

	return {};
}


bool
CompilerGLSL::shouldHaveVariableAssignment(const ShaderGraph::Expression& expr)
{
	auto successorCount = expr.shared_from_this().use_count() - 1;

	return visit(expr, Visitor
	{
		[&](const auto& e) { return successorCount > 1; },
		[](const InputAttributeExpression&)  { return false; },
		[](const OutputAttributeExpression&) { return true;  }, 
		[](const ParameterExpression&)       { return false; },
		[](const NativeFunctionExpression&)  { return true; },
	});
}


std::string
CompilerGLSL::buildVariableAssignments(const ShaderGraph::Expression& expr)
{
	return visit(expr, Visitor{
		[&](const auto& ex)
		{ 
			return std::format("{} {} = {};\n",
					           toString(expr.outputValueType()),
					           mNameLookUp[&expr],
					           format(&expr));
		},

		[&](const OutputAttributeExpression& ex)
		{
			return std::format("{} = {};\n",
							   mNameLookUp[&expr],
							   resolve(*ex.inputs().front()));
		},
	});
}


std::string
CompilerGLSL::formatFunctionArgumentList(const std::vector<const Expression*>& args)
{
	std::stringstream ss;

	for (auto [i, arg] : std::views::enumerate(args))
	{
		ss << resolve(*arg);

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
buildUniformCombinedTextureSamplerString(uint32_t set, uint32_t binding, const std::string& name, 
										 const Coral::CombinedTextureSamplerDefinition& definition)
{
	return std::format("layout (set = {}, binding = {}) uniform sampler2D {};\n", set, binding, name);
}


void
CompilerGLSL::createUniformBlockDefinitions()
{
	std::vector<const ShaderGraph::ParameterExpression*> parameters;
	std::unordered_set<const ShaderGraph::ParameterExpression*> inserted;
	for (auto shaderModule : { mVertexShader, mFragmentShader })
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
	
	auto samplers = parameters | std::views::filter([](auto p) { return p->outputValueType() == ShaderGraph::ValueType::SAMPLER2D; });
	auto uniforms = parameters | std::views::filter([](auto p) { return p->outputValueType() != ShaderGraph::ValueType::SAMPLER2D; });

	// Find all parameters that are not contained in a UniformBlock override
	for (auto parameter : uniforms)
	{
		if (!findUniformBinding(parameter->name()))
		{
			defaultUniformBlock.members.push_back(Coral::MemberDefinition{ convert(parameter->outputValueType()), 
																		   parameter->name(), 
																		   1 });
		}
	}

	// Add the default uniform block (if required) at the first unused binding
	if (!defaultUniformBlock.members.empty())
	{
		uint32_t binding{ 0 };
		for (; mDescriptorBindings.find(binding) != mDescriptorBindings.end(); ++binding) {}

		auto& descriptor	  = mDescriptorBindings[binding];
		descriptor.binding	  = binding;
		descriptor.definition = defaultUniformBlock;
		descriptor.name		  = mDefaultUniformBlockName;
	}

	// Add sampler parameters separately
	for (auto parameter : samplers)
	{
		uint32_t binding{ 0 };
		for (; mDescriptorBindings.find(binding) != mDescriptorBindings.end(); ++binding) {}

		auto& descriptor	  = mDescriptorBindings[binding];
		descriptor.binding	  = binding;
		descriptor.definition = Coral::CombinedTextureSamplerDefinition{};
		descriptor.name		  = parameter->name();
		binding++;
	}
}


bool
CompilerGLSL::createAttributeBindings()
{
	auto getAttributeLocation = [this](const std::string& name, const std::unordered_map<std::string, uint32_t>& attributeLookUp) mutable
	{
		auto iter = attributeLookUp.find(name);
		if (iter != attributeLookUp.end())
		{
			return iter->second;
		}

		uint32_t i = 0;
		while (true)
		{
			if (std::ranges::find_if(attributeLookUp, [=](const auto& pair) { return pair.second == i; }) == attributeLookUp.end())
			{
				return i;
			}
			i++;
		}

		return i;
	};

	auto shaderModules = std::array{ mVertexShader, mFragmentShader }
		| std::views::filter([&](auto shaderModule) { return shaderModule != nullptr; })
		| std::ranges::to<std::vector<const ShaderModule*>>();

	for (auto [i, shaderModule] : std::views::enumerate(shaderModules))
	{
		auto& bindings = mShaderStageAttributeBindingsLookUp[shaderModule];
		// Create the input location for the attribute bindings:
		// The input locations for the vertex shader are determined by `mInputAttributeBindingOverrides` if present.
		// Otherwise, default locations are created.
		// For the other shader stages we have to use the location of the same-named attribute of the previous shader
		// stage.
		if (i == 0)
		{
			bindings.inputAttributes = mInputAttributeBindingOverrides;

			for (const auto& attribute: shaderModule->inputs())
			{
				auto location = getAttributeLocation(attribute->attribute(), bindings.inputAttributes);
				bindings.inputAttributes[attribute->attribute()] = location;
			}
		}
		// Use the output locations of the previous shader stage for the input locations of the current stage
		else
		{
			auto prevShaderModule = shaderModules[i - 1];
			auto& prevBindings    = mShaderStageAttributeBindingsLookUp[prevShaderModule];
			for (auto attribute : shaderModule->inputs())
			{
				auto location = prevBindings.outputAttributes.find(attribute->attribute());

				if (location == prevBindings.outputAttributes.end())
				{
					return false;
				}

				bindings.inputAttributes[attribute->attribute()] = location->second;
			}
		}

		if (i == shaderModules.size() - 1)
		{
			bindings.outputAttributes = mOutputAttributeBindingOverrides;
		}
		// Create the output locations
		for (auto attribute : shaderModule->outputs())
		{
			std::visit(Visitor{
				[&](const std::string& attribute)
				{
					auto location = getAttributeLocation(attribute, bindings.outputAttributes);
					bindings.outputAttributes[attribute] = location;
				},
				[](auto) {}
				}, attribute->attribute());
		}
	}

	return true;
}


std::string
CompilerGLSL::buildUniformBlocksString(const ShaderModule& shaderModule)
{
	std::stringstream ss;

	auto parameters = shaderModule.parameters();

	for (const auto& [binding, descriptor] : mDescriptorBindings)
	{
		bool useBlock = std::ranges::any_of(parameters, [&](auto parameter)
		{
			return findUniformBinding(parameter->name()) == binding;
		});

		if (!useBlock)
		{
			continue;
		}

		std::visit(Visitor
		{
			[&](const Coral::UniformBlockDefinition& uniformBlock)
			{
				ss << buildUniformBlockString(0, descriptor.binding, descriptor.name, uniformBlock) << std::endl;
			},
			[&](auto)
			{
			},
			[&](const Coral::CombinedTextureSamplerDefinition& sampler)
			{
				ss << buildUniformCombinedTextureSamplerString(0, descriptor.binding, descriptor.name, sampler) << std::endl;
			}
		}, descriptor.definition);
		
	}

	return ss.str();
}


void
CompilerGLSL::buildVariableNames(const ShaderModule& shader)
{
	for (auto expr : shader.buildExpressionList())
	{
		if (!shouldHaveVariableAssignment(*expr))
		{
			continue;
		}

		visit(*expr, Visitor{
			[&](const OutputAttributeExpression& attr)
			{
				mNameLookUp[expr] = std::visit(Visitor{
						[](DefaultAttribute attribute) { return std::string(toString(attribute)); },
						[](const std::string& attribute) { return std::format("out_{}", attribute); },
					}, attr.attribute());
			},
			[&](const InputAttributeExpression& attr)
			{
				mNameLookUp[expr] = attr.attribute();
			},
			[&](const auto& e)
			{
				mNameLookUp[expr] = std::format("{}_{}", getTypeShortName(expr->outputValueType()), mNameLookUp.size());
			}
		});
	}
}


std::string
CompilerGLSL::buildInputAttributeDefinitionsString(const ShaderModule& shader)
{
	std::map<uint32_t, std::string> attributesSorted;
	for (const auto& attr : shader.inputs())
	{
		auto location = mShaderStageAttributeBindingsLookUp[&shader].inputAttributes[attr->attribute()];
		attributesSorted[location] = std::format("layout (location = {}) in {} {};\n",
			                                     location,
			                                     toString(attr->outputValueType()), format(*attr));
	}

	std::stringstream ss;
	for (auto [location, attr] : attributesSorted)
	{
		ss << attr;
	}

	return ss.str();
}


std::string
CompilerGLSL::buildOutputAttributeDefinitionsString(const ShaderModule& shader)
{
	std::map<uint32_t, std::string> attributesSorted;
	for (const auto& attr : shader.outputs())
	{
		if (auto attribute = std::get_if<std::string>(&attr->attribute()))
		{
			auto location = mShaderStageAttributeBindingsLookUp[&shader].outputAttributes[*attribute];
			attributesSorted[location] = std::format("layout (location = {}) out {} {};\n",
				                                     location,
				                                     toString(attr->outputValueType()), 
				                                     format(*attr));
		}
	}

	std::stringstream ss;
	for (auto [location, attr] : attributesSorted)
	{
		ss << attr;
	}

	return ss.str();
}


std::string
CompilerGLSL::buildMainFunctionString(const ShaderModule& shader)
{
	auto expressionList = shader.buildExpressionList();

	std::unordered_set<const Expression*> visited;

	std::stringstream ss;

	ss << "void main()" << std::endl;
	ss << "{" << std::endl;

	for (const auto& expr : expressionList)
	{
		if (visited.contains(expr))
		{
			continue;
		}

		if (!shouldHaveVariableAssignment(*expr))
		{
			continue;
		}

		ss << TAB << buildVariableAssignments(*expr);

		visited.insert(expr);
	}

	ss << "}";

	return ss.str();
}


Compiler&
CompilerGLSL::addShaderModule(Coral::ShaderStage stage, const ShaderModule& shaderModule)
{
	switch (stage)
	{
		case ShaderStage::VERTEX:	mVertexShader = &shaderModule; break;
		case ShaderStage::FRAGMENT: mFragmentShader = &shaderModule; break;
	}
	
	return *this;
}


Compiler&
CompilerGLSL::addUniformBlockOverride(uint32_t binding, std::string_view name, const Coral::UniformBlockDefinition& uniformBlock)
{
	auto& descriptor      = mDescriptorBindings[binding];
	descriptor.binding    = binding;
	descriptor.byteSize   = 0;
	descriptor.name		  = name;
	descriptor.definition = uniformBlock;

	return *this;
}


Compiler&
CompilerGLSL::addInputAttributeBindingLocation(uint32_t location, std::string_view name)
{
	mInputAttributeBindingOverrides[std::string(name)] = location;
	return *this;
}


Compiler&
CompilerGLSL::addOutputAttributeBindingLocation(uint32_t location, std::string_view name)
{
	mOutputAttributeBindingOverrides[std::string(name)] = location;
	return *this;
}


Compiler&
CompilerGLSL::setDefaultUniformBlockName(std::string_view name)
{
	mDefaultUniformBlockName = name;
	return *this;
}


std::expected<Compiler::Result, Compiler::Error>
CompilerGLSL::compile()
{
	if (!mVertexShader || ! mFragmentShader)
	{
		return std::unexpected(Compiler::Error{ "Missing shader definition" });
	}

	createUniformBlockDefinitions();

	if (!createAttributeBindings())
	{
		return std::unexpected(Compiler::Error{ "Shader attribute missmatch" });
	}

	Result result;

	for (auto [shaderModule, source] : { std::pair{ mVertexShader, &result.vertexShader},
									     std::pair{ mFragmentShader, &result.fragmentShader } })
	{
		buildVariableNames(*shaderModule);
		auto inputAttributes  = buildInputAttributeDefinitionsString(*shaderModule);
		auto outputAttributes = buildOutputAttributeDefinitionsString(*shaderModule);
		auto uniforms	      = buildUniformBlocksString(*shaderModule);
		auto mainFunc	      = buildMainFunctionString(*shaderModule);

		std::stringstream ss;
		ss << "#version 420" << std::endl;
		ss << inputAttributes << std::endl;
		ss << outputAttributes << std::endl;
		ss << uniforms << std::endl;
		ss << mainFunc << std::endl;

		*source = ss.str();

		std::cout << *source << std::endl;
	}

	return { std::move(result) };
}
