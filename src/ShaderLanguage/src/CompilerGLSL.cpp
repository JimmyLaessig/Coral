#include <Coral/ShaderLanguage/CompilerGLSL.hpp>

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

namespace
{

// Visitor class definition for std::visit
template<class... Ts>
struct Visitor : Ts...
{
	using Ts::operator()...;
};


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


//constexpr CoUniformFormat
//convert(ValueType typeId)
//{
//	switch (typeId)
//	{
//		case ValueType::BOOL:      return CO_UNIFORM_FORMAT_BOOL;
//		case ValueType::INT:       return CO_UNIFORM_FORMAT_INT32;
//		case ValueType::INT2:      return CO_UNIFORM_FORMAT_VEC2I;
//		case ValueType::INT3:      return CO_UNIFORM_FORMAT_VEC3I;
//		case ValueType::INT4:      return CO_UNIFORM_FORMAT_VEC4I;
//		case ValueType::FLOAT:     return CO_UNIFORM_FORMAT_FLOAT;
//		case ValueType::FLOAT2:    return CO_UNIFORM_FORMAT_VEC2F;
//		case ValueType::FLOAT3:    return CO_UNIFORM_FORMAT_VEC3F;
//		case ValueType::FLOAT4:    return CO_UNIFORM_FORMAT_VEC4F;
//		case ValueType::FLOAT3X3:  return CO_UNIFORM_FORMAT_MAT33F;
//		case ValueType::FLOAT4X4:  return CO_UNIFORM_FORMAT_MAT44F;
//		case ValueType::SAMPLER2D: assert(false); return CO_UNIFORM_FORMAT_INT32;
//	}
//
//	std::unreachable();
//}

 
//constexpr std::string_view
//toString(CoUniformFormat type)
//{
//	switch (type)
//	{
//		case CO_UNIFORM_FORMAT_BOOL:   return "bool";
//		case CO_UNIFORM_FORMAT_INT32:  return "int";
//		case CO_UNIFORM_FORMAT_FLOAT:  return "float";
//		case CO_UNIFORM_FORMAT_VEC2F:  return "vec2";
//		case CO_UNIFORM_FORMAT_VEC3F:  return "vec3";
//		case CO_UNIFORM_FORMAT_VEC4F:  return "vec4";
//		case CO_UNIFORM_FORMAT_VEC2I:  return "ivec2";
//		case CO_UNIFORM_FORMAT_VEC3I:  return "ivec3";
//		case CO_UNIFORM_FORMAT_VEC4I:  return "ivec4";
//		case CO_UNIFORM_FORMAT_MAT33F: return "mat3";
//		case CO_UNIFORM_FORMAT_MAT44F: return "mat4";
//	}
//
//	std::unreachable();
//}


constexpr std::string_view
toString(Operator op)
{
	switch (op)
	{
	    case Operator::ASSIGNMENT:       return "=";
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


bool isDefaultAttributeSemantic(std::string_view attributeName)
{
	return attributeName == DefaultAttributes::POSITION.value ||
		   attributeName == DefaultAttributes::DEPTH.value;
}


constexpr auto TAB = "    ";


template<typename Visitor>
auto visit(const Expression& expr, Visitor visitor)
{
	switch (expr.expressionType())
	{
		case ExpressionType::CONSTANT_FLOAT:   return visitor(static_cast<const ConstantExpression<float>&>(expr));
		case ExpressionType::CONSTANT_INT:     return visitor(static_cast<const ConstantExpression<int>&>(expr));
		case ExpressionType::CONSTANT_BOOL:    return visitor(static_cast<const ConstantExpression<bool>&>(expr));
		case ExpressionType::INPUT_ATTRIBUTE:  return visitor(static_cast<const InputAttributeExpression&>(expr));
		case ExpressionType::OUTPUT_ATTRIBUTE: return visitor(static_cast<const OutputAttributeExpression&>(expr));
		case ExpressionType::UNIFORM_BUFFER:   return visitor(static_cast<const UniformBufferExpression&>(expr));
		case ExpressionType::STRUCT_MEMBER:    return visitor(static_cast<const StructMemberExpression&>(expr));
		case ExpressionType::NATIVE_FUNCTION:  return visitor(static_cast<const NativeFunctionExpression&>(expr));
		case ExpressionType::CONSTRUCTOR:      return visitor(static_cast<const ConstructorExpression&>(expr));
		case ExpressionType::CAST:             return visitor(static_cast<const CastExpression&>(expr));
		case ExpressionType::SWIZZLE:          return visitor(static_cast<const SwizzleExpression&>(expr));
		case ExpressionType::CONDITIONAL:      return visitor(static_cast<const ConditionalExpression&>(expr));
		case ExpressionType::OPERATOR:         return visitor(static_cast<const OperatorExpression&>(expr));
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
	if (expr.attribute() == Coral::ShaderLanguage::DefaultAttributes::POSITION.value)
	{
		return "gl_Position";
	}

	if (expr.attribute() == Coral::ShaderLanguage::DefaultAttributes::DEPTH.value)
	{
		return "gl_FragDepth";
	}

	return std::format("out_{}", expr.attribute());
}


std::string
CompilerGLSL::format(const UniformBufferExpression& expr)
{
	return expr.name();
}


std::string
CompilerGLSL::format(const StructMemberExpression& expr)
{
	return expr.name();
}


std::string
CompilerGLSL::format(const OperatorExpression& expr)
{
	auto inputs = expr.inputs();
	if (expr.getOperator() == Operator::ASSIGNMENT)
	{
		return std::format("{} {} {}",
			resolve(*inputs[0]),
			toString(expr.getOperator()),
			resolve(*inputs[1]));
	}
	else
	{
		return std::format("({} {} {})",
			               resolve(*inputs[0]),
			               toString(expr.getOperator()),
			               resolve(*inputs[1]));
	}
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
					   toString(expr.valueTypeId()),
					   formatFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const CastExpression& expr)
{
	auto inputs = expr.inputs();
	return std::format("({}){}",
					   toString(expr.valueTypeId()),
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
CompilerGLSL::resolve(const Expression& expr)
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
	// TODO Fix
	/*for (const auto& [binding, descriptor] : mDescriptorBindings)
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
	}*/

	return {};
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


//std::string
//buildUniformBlockString(uint32_t set, uint32_t binding, const std::string& name, 
//	                    const CoUniformBlockDefinition& definition)
//{
//	std::stringstream ss;
//
//	ss << std::format("layout (std140, set = {}, binding = {}) uniform {}\n", set, binding, name);
//	ss << "{" << std::endl;
//
//	for (const auto& [type, name, count, size, paddedSize] : definition.members)
//	{
//		ss << TAB << toString(type) << " " << name << ";" << std::endl;
//	}
//
//	ss << "};" << std::endl;
//
//	return ss.str();
//}


//std::string
//buildUniformCombinedTextureSamplerString(uint32_t set, uint32_t binding, const std::string& name, 
//										 const CoCombinedTextureSamplerDefinition& definition)
//{
//	return std::format("layout (set = {}, binding = {}) uniform sampler2D {};\n", set, binding, name);
//}


void
CompilerGLSL::createUniformBlockDefinitions()
{
	//std::vector<const ParameterExpression*> parameters;
	//std::unordered_set<const ParameterExpression*> inserted;
	//for (auto shaderModule : { mVertexShader, mFragmentShader })
	//{
	//	if (!shaderModule)
	//	{
	//		continue;
	//	}

	//	for (auto p : shaderModule->parameters())
	//	{
	//		if (inserted.insert(p).second)
	//		{
	//			parameters.push_back(p);
	//		}
	//	}
	//}

	//CoUniformBlockDefinition defaultUniformBlock{};
	//
	//auto samplers = parameters | std::views::filter([](auto p) { return p->valueTypeId() == ValueType::SAMPLER2D; });
	//auto uniforms = parameters | std::views::filter([](auto p) { return p->valueTypeId() != ValueType::SAMPLER2D; });

	//// Find all parameters that are not contained in a UniformBlock override
	//for (auto parameter : uniforms)
	//{
	//	if (!findUniformBinding(parameter->name()))
	//	{
	//		defaultUniformBlock.members.push_back(Coral::MemberDefinition{ convert(parameter->valueTypeId()), 
	//																	   parameter->name(), 
	//																	   1 });
	//	}
	//}

	//// Add the default uniform block (if required) at the first unused binding
	//if (!defaultUniformBlock.members.empty())
	//{
	//	uint32_t binding{ 0 };
	//	for (; mDescriptorBindings.find(binding) != mDescriptorBindings.end(); ++binding) {}

	//	auto& descriptor	  = mDescriptorBindings[binding];
	//	descriptor.binding	  = binding;
	//	descriptor.definition = defaultUniformBlock;
	//	descriptor.name		  = mDefaultUniformBlockName;
	//}

	//// Add sampler parameters separately
	//for (auto parameter : samplers)
	//{
	//	uint32_t binding{ 0 };
	//	for (; mDescriptorBindings.find(binding) != mDescriptorBindings.end(); ++binding) {}

	//	auto& descriptor	  = mDescriptorBindings[binding];
	//	descriptor.binding	  = binding;
	//	descriptor.definition = Coral::CombinedTextureSamplerDefinition{};
	//	descriptor.name		  = parameter->name();
	//	binding++;
	//}
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
			auto location = getAttributeLocation(attribute->attribute(), bindings.outputAttributes);
			bindings.outputAttributes[attribute->attribute()] = location;
		}
	}

	return true;
}


std::string
CompilerGLSL::buildUniformBlocksString(const ShaderModule& shaderModule)
{
	std::stringstream ss;

	/*auto parameters = shaderModule.parameters();

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
			[&](auto) {},
			[&](const Coral::CombinedTextureSamplerDefinition& sampler)
			{
				ss << buildUniformCombinedTextureSamplerString(0, descriptor.binding, descriptor.name, sampler) << std::endl;
			}
		}, descriptor.definition);
		
	}*/

	return ss.str();
}


std::string
CompilerGLSL::buildInputAttributeDefinitionsString(const ShaderModule& shader)
{
	std::map<uint32_t, std::string> attributesSorted;
	for (const auto& attr : shader.inputs())
	{
		if (isDefaultAttributeSemantic(attr->attribute()))
		{
			continue;
		}
		auto location = mShaderStageAttributeBindingsLookUp[&shader].inputAttributes[attr->attribute()];
		attributesSorted[location] = std::format("layout (location = {}) in {} {};\n",
			                                     location,
			                                     toString(attr->valueTypeId()), format(*attr));
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
		if (isDefaultAttributeSemantic(attr->attribute()))
		{
			continue;
		}

		auto location = mShaderStageAttributeBindingsLookUp[&shader].outputAttributes[attr->attribute()];
		attributesSorted[location] = std::format("layout (location = {}) out {} {};\n",
			                                     location,
			                                     toString(attr->valueTypeId()),
			                                     format(*attr));
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
	std::stringstream ss;

	ss << "void main()" << std::endl;
	ss << "{" << std::endl;

	for (const auto expr : shader.expressionList())
	{
		if (expr->cast<InputAttributeExpression>()  || 
			expr->cast<OutputAttributeExpression>() || 
			expr->cast<UniformBufferExpression>()   || 
			expr->cast<StructMemberExpression>())
		{
			continue;
		}

		if (auto op = expr->cast<OperatorExpression>())
		{
			if (op->getOperator() == Operator::ASSIGNMENT)
			{
				ss << TAB << format(expr) << ";\n";
			}

			continue;
		}

		if (!expr->inlineIfPossible())
		{
			mNameLookUp[expr] = std::format("{}{}", getTypeShortName(expr->valueTypeId()), mVarCounter++);
			ss << TAB << std::format("{} {} = {};\n", toString(expr->valueTypeId()), mNameLookUp[expr], format(expr));
			continue;
		}
	}

	ss << "}";

	return ss.str();
}


//Compiler&
//CompilerGLSL::addUniformBlockOverride(uint32_t binding, std::string_view name, const CoUniformBlockDefinition& uniformBlock)
//{
//	auto& descriptor      = mDescriptorBindings[binding];
//	descriptor.binding    = binding;
//	descriptor.byteSize   = 0;
//	descriptor.name		  = name;
//	descriptor.definition = uniformBlock;
//
//	return *this;
//}


//Compiler&
//CompilerGLSL::addInputAttributeBindingLocation(uint32_t location, std::string_view name)
//{
//	mInputAttributeBindingOverrides[std::string(name)] = location;
//	return *this;
//}


//Compiler&
//CompilerGLSL::addOutputAttributeBindingLocation(uint32_t location, std::string_view name)
//{
//	mOutputAttributeBindingOverrides[std::string(name)] = location;
//	return *this;
//}


//Compiler&
//CompilerGLSL::setDefaultUniformBlockName(std::string_view name)
//{
//	mDefaultUniformBlockName = name;
//	return *this;
//}


std::expected<Compiler::Result, Compiler::Error>
CompilerGLSL::Compile(const ShaderModule& vertexShader, const ShaderModule& fragmentShader)
{
	mVertexShader   = &vertexShader;
	mFragmentShader = &fragmentShader;

	createUniformBlockDefinitions();

	if (!createAttributeBindings())
	{
		return std::unexpected(Compiler::Error{ "Shader attribute missmatch" });
	}

	Result result;

	for (auto [shaderModule, source] : { std::pair{ mVertexShader, &result.vertexShader},
									     std::pair{ mFragmentShader, &result.fragmentShader } })
	{
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
