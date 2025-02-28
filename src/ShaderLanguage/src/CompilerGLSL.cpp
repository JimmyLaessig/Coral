#include <Coral/ShaderGraph/CompilerGLSL.hpp>

#include "Visitor.hpp"

#include <cassert>
#include <format>
#include <iostream>
#include <map>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>


using namespace Coral::ShaderLanguage::ShaderGraph;

namespace
{

constexpr std::string_view
getTypeShortName(ValueTypeId typeId)
{
	switch (typeId)
	{
		case ValueTypeId::BOOL:    	 return "b";
		case ValueTypeId::INT:       return "i";
		case ValueTypeId::INT2:      return "iv";
		case ValueTypeId::INT3:      return "iv";
		case ValueTypeId::INT4:      return "iv";
		case ValueTypeId::FLOAT:     return "f";
		case ValueTypeId::FLOAT2:    return "v";
		case ValueTypeId::FLOAT3:    return "v";
		case ValueTypeId::FLOAT4:    return "v";
		case ValueTypeId::FLOAT3X3:  return "m";
		case ValueTypeId::FLOAT4X4:  return "m";
		case ValueTypeId::SAMPLER2D: return "s";
	}

	std::unreachable();
}


constexpr std::string_view
toString(ValueTypeId typeId)
{
	switch (typeId)
	{
		case ValueTypeId::BOOL:      return "bool";
		case ValueTypeId::INT:       return "int";
		case ValueTypeId::INT2:      return "ivec2";
		case ValueTypeId::INT3:      return "ivec3";
		case ValueTypeId::INT4:      return "ivec5";
		case ValueTypeId::FLOAT:     return "float";
		case ValueTypeId::FLOAT2:    return "vec2";
		case ValueTypeId::FLOAT3:    return "vec3";
		case ValueTypeId::FLOAT4:    return "vec4";
		case ValueTypeId::FLOAT3X3:  return "mat3";
		case ValueTypeId::FLOAT4X4:  return "mat4";
		case ValueTypeId::SAMPLER2D: return "sampler2D";
	}

	std::unreachable();
}


constexpr Coral::ValueType
convert(ValueTypeId typeId)
{
	switch (typeId)
	{
		case ValueTypeId::BOOL:      return Coral::ValueType::BOOL;
		case ValueTypeId::INT:       return Coral::ValueType::INT;
		case ValueTypeId::INT2:      return Coral::ValueType::VEC2I;
		case ValueTypeId::INT3:      return Coral::ValueType::VEC3I;
		case ValueTypeId::INT4:      return Coral::ValueType::VEC4I;
		case ValueTypeId::FLOAT:     return Coral::ValueType::FLOAT;
		case ValueTypeId::FLOAT2:    return Coral::ValueType::VEC2F;
		case ValueTypeId::FLOAT3:    return Coral::ValueType::VEC3F;
		case ValueTypeId::FLOAT4:    return Coral::ValueType::VEC4F;
		case ValueTypeId::FLOAT3X3:  return Coral::ValueType::MAT33F;
		case ValueTypeId::FLOAT4X4:  return Coral::ValueType::MAT44F;
		case ValueTypeId::SAMPLER2D: assert(false); return Coral::ValueType::INT;
	}

	std::unreachable();
}


constexpr std::string_view
toString(Coral::ValueType type)
{
	switch (type)
	{
		case Coral::ValueType::BOOL:   return "bool";
		case Coral::ValueType::INT:	   return "int";
		case Coral::ValueType::FLOAT:  return "float";
		case Coral::ValueType::VEC2F:  return "vec2";
		case Coral::ValueType::VEC3F:  return "vec3";
		case Coral::ValueType::VEC4F:  return "vec4";
		case Coral::ValueType::VEC2I:  return "ivec2";
		case Coral::ValueType::VEC3I:  return "ivec3";
		case Coral::ValueType::VEC4I:  return "ivec4";
		case Coral::ValueType::MAT33F: return "mat3";
		case Coral::ValueType::MAT44F: return "mat4";
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
					   toString(expr.outputShaderTypeId()),
					   getRefName(expr),
					   getRefName(lhs),
					   toString(expr.getOperator()),
					   getRefName(rhs));
}


std::string
CompilerGLSL::format(const NativeFunctionExpression& expr)
{
	return std::format("{} {} = {}({});\n",
					   toString(expr.outputShaderTypeId()),
					   getRefName(expr),
					   expr.functionName(),
					   buildFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const ConstructorExpression& expr)
{
	return std::format("{} {} = {}({});\n",
					   toString(expr.outputShaderTypeId()),
					   getRefName(expr),
					   toString(expr.outputShaderTypeId()),
					   buildFunctionArgumentList(expr.inputs()));
}


std::string
CompilerGLSL::format(const CastExpression& expr)
{
	assert(expr.inputs().size() == 1);
	return std::format("{} {} = ({}){};\n",
					   toString(expr.outputShaderTypeId()),
					   getRefName(expr),
					   toString(expr.outputShaderTypeId()),
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

	auto varName = std::format("{}{}",getTypeShortName(expr.outputShaderTypeId()), mNameLookUp.size());
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

	for (const auto& [type, name, count] : definition.members)
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
	
	// Find all parameters that are contained in a UniformBlock override
	for (auto parameter : parameters)
	{
		Visitor v{
			[](const auto& definition) { return false; },
			[&](const Coral::UniformBlockDefinition& definition)
			{
				return std::ranges::find_if(definition.members, [&](const auto& member)
					{
						return member.name == parameter->name();
					}) != definition.members.end();
			}
		};

		auto view = std::views::values(mDescriptorBindings);
		auto block = std::ranges::find_if(view,
			[&](const auto& binding) { return std::visit(v, binding.definition); });
		
		if (block == view.end() && parameter->outputShaderTypeId() != ValueTypeId::SAMPLER2D)
		{
			defaultUniformBlock.members.push_back(Coral::MemberDefinition{ convert(parameter->outputShaderTypeId()), parameter->name(), 1 });
		}
	}

	// Add the default uniform block (if required) at the first unused binding
	if (!defaultUniformBlock.members.empty())
	{
		uint32_t binding{ 0 };
		for (; mDescriptorBindings.find(binding) != mDescriptorBindings.end(); ++binding) {}

		Coral::DescriptorBindingDefinition definition;
		definition.binding	         = binding;
		definition.definition        = defaultUniformBlock;
		definition.name		         = mDefaultUniformBlockName;
		definition.set				 = 0;
		mDescriptorBindings[binding] = std::move(definition);
	}

	// Add sampler parameters separately
	for (auto parameter : parameters)
	{
		if (parameter->outputShaderTypeId() == ValueTypeId::SAMPLER2D)
		{
			uint32_t binding{ 0 };
			for (; mDescriptorBindings.find(binding) != mDescriptorBindings.end(); ++binding) {}

			Coral::DescriptorBindingDefinition definition;
			definition.binding			 = binding;
			definition.definition		 = Coral::CombinedTextureSamplerDefinition{};
			definition.name				 = parameter->name();
			definition.set				 = 0;
			mDescriptorBindings[binding] = std::move(definition);
			binding++;
		}
	}
	return true;
}


bool
CompilerGLSL::createAttributeLocationDefinitions()
{
	auto shaderModules = std::array{ mShaderProgram->vertexShader(), mShaderProgram->fragmentShader() }
		| std::views::filter([&](auto shaderModule) { return shaderModule != nullptr; })
		| std::ranges::to<std::vector>();

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
					[&](const auto& pair) { return pair.first->name() == attribute->name() && pair.first->outputShaderTypeId() == attribute->outputShaderTypeId(); });

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

	uint32_t set = 0;

	for (const auto& [binding, definition] : mDescriptorBindings)
	{
		std::visit(Visitor
		{
			[&](const Coral::UniformBlockDefinition& uniformBlock)
			{
				ss << buildUniformBlockString(definition.set, definition.binding, definition.name, uniformBlock) << std::endl;
			},
			[&](const auto& uniformBlock)
			{
			},
			[&](const Coral::CombinedTextureSamplerDefinition& sampler)
			{
				ss << buildUniformCombinedTextureSamplerString(definition.set, definition.binding, definition.name, sampler) << std::endl;
			}

			},
			definition.definition);

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
						  toString(attr->outputShaderTypeId()),
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
						  toString(attr->outputShaderTypeId()),
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
		if (output->name() == DefaultSemantics::POSITION && shaderModule.shaderStage() == ShaderStage::VERTEX)
		{
			ss << TAB << std::format("gl_Position = {};", getRefName(output->inputs().front())) << std::endl;
		}
		if (output->name() == DefaultSemantics::DEPTH && shaderModule.shaderStage() == ShaderStage::FRAGMENT)
		{
			ss << TAB << std::format("gl_FragDepth = {};", getRefName(output->inputs().front())) << std::endl;
		}
	}

	ss << "}";

	return ss.str();
}


Compiler&
CompilerGLSL::setShaderProgram(const ShaderProgram& shaderModule)
{
	mShaderProgram = &shaderModule;
	return *this;
}


Compiler&
CompilerGLSL::addUniformBlockOverride(const Coral::UniformBlockDefinition& uniformBlock)
{
	//mUniformBlocks[uniformBlock.binding] = uniformBlock;
	return *this;
}


Compiler&
CompilerGLSL::setDefaultUniformBlockName(std::string_view name)
{
	mDefaultUniformBlockName = name;
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
			std::cout << *source << std::endl;
		}
	}

	return result;
}
