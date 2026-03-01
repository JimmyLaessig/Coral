#include <Coral/ShaderLanguage/CompilerGLSL.hpp>

#include <Coral/ShaderLanguage/Visitor.hpp>
#include <cassert>
#include <cmath>
#include <format>
#include <iostream>
#include <map>
#include <ranges>
#include <set>
#include <sstream>
#include <string_view>
#include <utility>


using namespace Coral::ShaderLanguage;

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
toString(NativeFunction func)
{
	switch (func)
	{
		case NativeFunction::NORMALIZE:    return "normalize";
		case NativeFunction::DOT:          return "dot";
		case NativeFunction::CROSS:        return "cross";
		case NativeFunction::LENGTH:       return "length";
		case NativeFunction::DISTANCE:     return "distance";
		case NativeFunction::SAMPLE:       return "texture";
		case NativeFunction::TEXTURE_SIZE: return "textureSize";
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


template<typename Visitor>
auto visit(const Expression& expr, Visitor visitor)
{
	switch (expr.Type())
	{
		case ExpressionType::CONSTANT_FLOAT:   return visitor(static_cast<const ConstantExpression<float>&>(expr));
		case ExpressionType::CONSTANT_INT:     return visitor(static_cast<const ConstantExpression<int>&>(expr));
		case ExpressionType::CONSTANT_BOOL:    return visitor(static_cast<const ConstantExpression<bool>&>(expr));
		case ExpressionType::INPUT_ATTRIBUTE:  return visitor(static_cast<const InputAttributeExpression&>(expr));
		case ExpressionType::OUTPUT_ATTRIBUTE: return visitor(static_cast<const OutputAttributeExpression&>(expr));
		case ExpressionType::UNIFORM_BUFFER:   return visitor(static_cast<const UniformBufferExpression&>(expr));
		case ExpressionType::UNIFORM:          return visitor(static_cast<const UniformExpression&>(expr));
		case ExpressionType::NATIVE_FUNCTION:  return visitor(static_cast<const NativeFunctionExpression&>(expr));
		case ExpressionType::CONSTRUCTOR:      return visitor(static_cast<const ConstructorExpression&>(expr));
		case ExpressionType::CAST:             return visitor(static_cast<const CastExpression&>(expr));
		case ExpressionType::SWIZZLE:          return visitor(static_cast<const SwizzleExpression&>(expr));
		//case ExpressionType::CONDITIONAL:      return visitor(static_cast<const ConditionalExpression&>(expr));
		case ExpressionType::OPERATOR:         return visitor(static_cast<const OperatorExpression&>(expr));
		case ExpressionType::SAMPLER:          return visitor(static_cast<const SamplerExpression&>(expr));
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
	return expr.name();
}


std::string
CompilerGLSL::format(const OutputAttributeExpression& expr)
{
	return std::visit(Visitor{
		[](DefaultSemantics semantic) -> std::string
		{
			switch (semantic)
			{
				case DefaultSemantics::SV_POSITION: return "gl_Position";
				case DefaultSemantics::SV_DEPTH:    return "gl_FragDepth";
				default: assert(false); return "";
			}
		},
		[](const AttributeBinding& binding)
		{
			return std::format("out_{}", binding.name);
		} }, expr.BindingInfo());
}


std::string
CompilerGLSL::format(const UniformBufferExpression& expr)
{
	return expr.name();
}


std::string
CompilerGLSL::format(const UniformExpression& expr)
{
	return expr.name();
}


std::string
CompilerGLSL::format(const OperatorExpression& expr)
{
	auto inputs = expr.Inputs();
	if (expr.getOperator() == Operator::ASSIGNMENT)
	{
		auto lhs = resolve(*inputs[0]);
		mNameLookUp[&expr] = lhs;
		return std::format("{} {} {}",
			               lhs,
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
		               toString(expr.Function()),
					   formatFunctionArgumentList(expr.Inputs()));
}


std::string
CompilerGLSL::format(const ConstructorExpression& expr)
{

	return std::format("{}({})",
					   toString(expr.GetValueType()),
					   formatFunctionArgumentList(expr.Inputs()));
}


std::string
CompilerGLSL::format(const CastExpression& expr)
{
	auto inputs = expr.Inputs();
	return std::format("({}){}",
					   toString(expr.GetValueType()),
					   resolve(*inputs.front()));
}


std::string
CompilerGLSL::format(const SwizzleExpression& expr)
{
	auto inputs = expr.Inputs();
	return std::format("{}.{}", 
		               resolve(*inputs.front()),
	                   toString(expr.swizzle()));
}


//std::string
//CompilerGLSL::format(const ConditionalExpression& expr)
//{
//	auto inputs = expr.inputs();
//	return std::format("({} ? {} : {})",
//					   resolve(*inputs[0]),
//					   resolve(*inputs[1]),
//					   resolve(*inputs[2]));
//}


std::string
CompilerGLSL::format(const SamplerExpression& expr)
{
	return expr.name();
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
CompilerGLSL::buildUniformBlocksString()
{
	std::stringstream ss;

	std::unordered_map<const UniformBufferExpression*, std::vector<const UniformExpression*>> expressionsSorted;

	for (const auto& expression : mInstructionsList)
	{
		if (auto uniform = expression->Cast<UniformExpression>())
		{
			for (auto input : uniform->Inputs())
			{
				if (auto uniformBuffer = input->Cast<UniformBufferExpression>())
				{
					expressionsSorted[uniformBuffer].push_back(uniform);
				}
			}
		}
	}

	for (const auto& [buffer, members] : expressionsSorted)
	{
		ss << std::format("layout (std140, set = {}, binding = {}) uniform {}\n", 0, buffer->location(), buffer->name());
		ss << "{\n";

		for (const auto& member : members)
		{
			ss << TAB << toString(member->GetValueType()) << " " << member->name() << ";\n";
		}

		ss << "};\n";
		ss << "\n";
	}

	return ss.str();
}


std::string
CompilerGLSL::buildSamplerString()
{
	std::stringstream ss;

	std::set<const SamplerExpression*> samplers;

	for (const auto& expression : mInstructionsList)
	{
		if (auto sampler = expression->Cast<SamplerExpression>())
		{
			samplers.insert(sampler);
		}
	}

	for (auto sampler : samplers)
	{
		ss << std::format("layout(binding = {}) uniform sampler2D {};\n", sampler->location(), sampler->name());
	}

	ss << "\n";

	return ss.str();
}


std::string
CompilerGLSL::buildInputAttributeDefinitionsString()
{
	std::map<uint32_t, std::string> attributesSorted;
	for (const auto& attr : mShader->Inputs())
	{
		auto location = attr->location();
		attributesSorted[location] = std::format("layout (location = {}) in {} {};\n",
			                                     location,
			                                     toString(attr->GetValueType()), format(*attr));
	}

	std::stringstream ss;
	for (auto [location, attr] : attributesSorted)
	{
		ss << attr;
	}

	ss << "\n";
	return ss.str();
}


std::string
CompilerGLSL::buildOutputAttributeDefinitionsString()
{
	std::map<uint32_t, std::string> attributesSorted;
	for (const auto& attr : mShader->Outputs())
	{
		std::visit(Visitor{
			[](DefaultSemantics)
			{
			}, 
			[&](const AttributeBinding& binding)
			{
				attributesSorted[binding.location] = std::format("layout (location = {}) out {} {};\n",
					                                             binding.location,
													             toString(attr->GetValueType()),
													             format(*attr));
			}
		}, attr->BindingInfo());
	}

	std::stringstream ss;
	for (auto [location, attr] : attributesSorted)
	{
		ss << attr;
	}

	return ss.str();
}


std::string
CompilerGLSL::buildMainFunctionString()
{
	std::stringstream ss;

	ss << "void main()" << std::endl;
	ss << "{" << std::endl;

	for (const auto expr : mInstructionsList)
	{
		if (expr->Cast<InputAttributeExpression>()  || 
			expr->Cast<OutputAttributeExpression>() || 
			expr->Cast<UniformBufferExpression>()   || 
			expr->Cast<UniformExpression>()         ||
			expr->Cast<SamplerExpression>())
		{
			continue;
		}

		if (auto op = expr->Cast<OperatorExpression>())
		{
			if (op->getOperator() == Operator::ASSIGNMENT)
			{
				ss << TAB << format(expr) << ";\n";
			}

			continue;
		}

		if (!expr->InlineIfPossible())
		{
			mNameLookUp[expr] = std::format("{}{}", getTypeShortName(expr->GetValueType()), mVarCounter++);
			ss << TAB << std::format("{} {} = {};\n", toString(expr->GetValueType()), mNameLookUp[expr], format(expr));
			continue;
		}
	}

	auto outputs = mShader->Outputs();
	for (const auto& output : outputs)
	{
		auto input = output->Inputs().front();
		if (input->InlineIfPossible())
		{
			ss << TAB << std::format("{} = {};\n", format(output), format(input));
		}
		else
		{
			ss << TAB << std::format("{} = {};\n", format(output), mNameLookUp[input]);
		}
	}

	ss << "}";

	return ss.str();
}


std::expected<Compiler::Result, Compiler::Error>
CompilerGLSL::Compile(const ShaderGraph& shaderModule, ShaderStage stage)
{
	mShader = &shaderModule;
	
	mInstructionsList = mShader->ExpressionList();

	Result result;

	auto inputAttributes  = buildInputAttributeDefinitionsString();
	auto outputAttributes = buildOutputAttributeDefinitionsString();
	auto uniforms	      = buildUniformBlocksString();
	auto samplers         = buildSamplerString();
	auto mainFunc	      = buildMainFunctionString();

	std::stringstream ss;
	ss << "#version 420" << std::endl;
	ss << inputAttributes << std::endl;
	ss << outputAttributes << std::endl;
	ss << uniforms << std::endl;
	ss << samplers << std::endl;
	ss << mainFunc << std::endl;

	result.shaderCode = ss.str();

	return { std::move(result) };
}
