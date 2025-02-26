#include "ShaderModuleImpl.hpp"

#include <spirv_reflect.h>

#include <algorithm>
#include <assert.h>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <bitset>

using namespace Coral::Vulkan;

std::optional<Coral::AttributeFormat>
convert(SpvReflectFormat format)
{
	switch (format)
	{
		case SPV_REFLECT_FORMAT_R16_UINT:				return Coral::AttributeFormat::UINT16;
		case SPV_REFLECT_FORMAT_R32_UINT:				return Coral::AttributeFormat::UINT32;
		case SPV_REFLECT_FORMAT_R16_SINT:				return Coral::AttributeFormat::INT16;
		case SPV_REFLECT_FORMAT_R32_SINT:				return Coral::AttributeFormat::INT32;
		case SPV_REFLECT_FORMAT_R32_SFLOAT:				return Coral::AttributeFormat::FLOAT;
		case SPV_REFLECT_FORMAT_R32G32_SFLOAT:			return Coral::AttributeFormat::VEC2;
		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:		return Coral::AttributeFormat::VEC3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:	return Coral::AttributeFormat::VEC4;
		default:
			return {};
	}
}


void
insertUniformBlockBindingRecursive(const SpvReflectBlockVariable& variable, const std::string& parentName, Coral::UniformBlockDefinition& result)
{
	/*assert(binding.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	assert(binding.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT);*/
	std::string fullName = parentName + variable.name;
	
	auto& traits = variable.type_description->traits;

	constexpr auto BoolFlags = SPV_REFLECT_TYPE_FLAG_BOOL;
	constexpr auto IntFlags = SPV_REFLECT_TYPE_FLAG_INT;
	constexpr auto FloatFlags = SPV_REFLECT_TYPE_FLAG_FLOAT;
	constexpr auto MatrixFlags = SPV_REFLECT_TYPE_FLAG_MATRIX | SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_FLOAT;
	constexpr auto FloatVectorFlags = SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_FLOAT;
	constexpr auto IntVectorFlags = SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_INT;
	constexpr auto StructFlags = SPV_REFLECT_TYPE_FLAG_STRUCT | SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK;
	switch (variable.type_description->type_flags)
	{
	case BoolFlags:
		result.members.push_back({ Coral::ValueType::BOOL, fullName , 1 });
		break;
	case IntFlags:
		result.members.push_back({ Coral::ValueType::INT, fullName , 1 });
		break;
	case FloatFlags:
		result.members.push_back({ Coral::ValueType::FLOAT, fullName , 1 });
		break;
	case FloatVectorFlags:
		switch (traits.numeric.vector.component_count)
		{
		case 2:
			result.members.push_back({ Coral::ValueType::VEC2F, fullName , 1 });
			break;
		case 3:
			result.members.push_back({ Coral::ValueType::VEC3F, fullName , 1 });
			break;
		case 4:
			result.members.push_back({ Coral::ValueType::VEC4F, fullName , 1 });
			break;
		default:
			assert(false);
		}
		break;
	case IntVectorFlags:
		switch (traits.numeric.vector.component_count)
		{
		case 2:
			result.members.push_back({ Coral::ValueType::VEC2I, fullName , 1 });
			break;
		case 3:
			result.members.push_back({ Coral::ValueType::VEC3I, fullName , 1 });
			break;
		case 4:
			result.members.push_back({ Coral::ValueType::VEC4I, fullName , 1 });
			break;
		default:
			assert(false);
		}
		break;
	case MatrixFlags:
		if (traits.numeric.matrix.column_count == 4 && traits.numeric.matrix.row_count == 4)
		{
			result.members.push_back({ Coral::ValueType::MAT44F, fullName , 1 });
		}
		else if (traits.numeric.matrix.column_count == 3 && traits.numeric.matrix.row_count == 3)
		{
			result.members.push_back({ Coral::ValueType::MAT33F, fullName , 1 });
		}
		else
		{
			assert(false);
		}
		break;
	case StructFlags:
	{
		for (const auto& member : std::span<SpvReflectBlockVariable>{ variable.members, variable.member_count })
		{
			insertUniformBlockBindingRecursive(member, fullName + ".", result);
		}

		break;
	}
	default:

		assert(false);
	}
}


Coral::UniformBlockDefinition
createUniformBlockBinding(const SpvReflectDescriptorBinding& binding)
{
	Coral::UniformBlockDefinition result;
	assert(binding.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	assert(binding.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT);

	for (const auto& member : std::span{ binding.block.members, binding.block.member_count })
	{
		insertUniformBlockBindingRecursive(member, "", result);
	}

	return result;
}


bool
ShaderModuleImpl::reflect(std::span<const std::byte> spirvCode)
{
	SpvReflectShaderModule module{};

	if (spvReflectCreateShaderModule(spirvCode.size_bytes(), spirvCode.data(), &module) != SPV_REFLECT_RESULT_SUCCESS)
	{
		return false;
	}

	//Finally finally = [&] { spvReflectDestroyShaderModule(&module); };

	if (module.entry_point_name == nullptr)
	{
		return false;
	}

	std::vector<SpvReflectDescriptorSet*> sets;
	{
		uint32_t count{ 0 };
		spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
		sets.resize(count);
		spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
	}

	for (auto set : sets)
	{
		for (auto binding : std::span{ set->bindings, set->binding_count })
		{	
			auto& descriptorBinding	  = mDescriptorBindings.emplace_back();
			descriptorBinding.set	  = set->set;
			descriptorBinding.binding = binding->binding;

			switch (binding->descriptor_type)
			{
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					descriptorBinding.name		 = binding->type_description->type_name;
					descriptorBinding.definition = createUniformBlockBinding(*binding);
					break;
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
					descriptorBinding.name = binding->name;
					descriptorBinding.definition.emplace<Coral::TextureDefinition>();
					break;
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
					descriptorBinding.name		 = binding->name;
					descriptorBinding.definition.emplace<Coral::SamplerDefinition>();
					break;
				case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					descriptorBinding.name = binding->name;
					descriptorBinding.definition.emplace<Coral::CombinedTextureSamplerDefinition>();
					break;
				default:
					assert(false);
			}
		}
	}

	std::vector<SpvReflectInterfaceVariable*> inputVariables;
	{
		uint32_t count{ 0 };
		spvReflectEnumerateInputVariables(&module, &count, nullptr);
		inputVariables.resize(count);
		spvReflectEnumerateInputVariables(&module, &count, inputVariables.data());
	}

	for (auto variable : inputVariables)
	{
		auto format = convert(variable->format);
		if (!format)
		{
			// TODO Log unsupported input format
			assert(false);
			return false;
		}

		auto& description		= mInputDescriptions.emplace_back();
		description.location	= variable->location;
		description.name		= variable->name;
		description.format		= *format;
	}

	std::sort(mInputDescriptions.begin(), mInputDescriptions.end(), 
			  [](const AttributeBindingDescription& lhs, const AttributeBindingDescription& rhs) { return lhs.location < rhs.location; });

	uint32_t binding{ 0 };
	for (auto& description : mInputDescriptions)
	{
		description.binding = binding++;
	}

	std::vector<SpvReflectInterfaceVariable*> outputVariables;
	{
		uint32_t count{ 0 };
		spvReflectEnumerateOutputVariables(&module, &count, nullptr);
		outputVariables.resize(count);
		spvReflectEnumerateOutputVariables(&module, &count, outputVariables.data());
	}

	for (auto variable : outputVariables)
	{
		if (variable->format == SPV_REFLECT_FORMAT_UNDEFINED)
		{
			continue;
		}

		auto format = convert(variable->format);
		if (!format)
		{
			// TODO Log unsupported format
			assert(false);
			return false;
		}

		auto& description		= mOutputDescriptions.emplace_back();
		description.location	= variable->location;
		description.name		= variable->name;
		description.format		= *format;
	}

	return true;
}


std::optional<Coral::ShaderModuleCreationError>
ShaderModuleImpl::init(ContextImpl& context, const ShaderModuleCreateConfig& config)
{
	mContext		= &context;
	mName			= config.name;
	mShaderStage	= config.stage;
	mEntryPoint		= config.entryPoint;

	VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.pCode	= (uint32_t*)config.source.data();
	createInfo.codeSize = config.source.size();
	
	if (vkCreateShaderModule(mContext->getVkDevice(), &createInfo, nullptr, &mShaderModule) != VK_SUCCESS)
	{
		return ShaderModuleCreationError::INTERNAL_ERROR;
	}

	if (!reflect(config.source))
	{
		return ShaderModuleCreationError::INTERNAL_ERROR;
	}

	return {};
}


ShaderModuleImpl::~ShaderModuleImpl()
{
	if (mContext && mShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(mContext->getVkDevice(), mShaderModule, nullptr);
	}
}


const std::string&
ShaderModuleImpl::name() const
{
	return mName;
}


Coral::ShaderStage
ShaderModuleImpl::shaderStage() const
{
	return mShaderStage;
}


const std::string&
ShaderModuleImpl::entryPoint() const
{
	return mEntryPoint;
}


std::span<const Coral::AttributeBindingDescription>
ShaderModuleImpl::inputAttributeBindingDefinitions() const
{
	return mInputDescriptions;
}


std::span<const Coral::AttributeBindingDescription>
ShaderModuleImpl::outputAttributeBindingDefinitions() const
{
	return mOutputDescriptions;
}


std::span<const Coral::DescriptorBindingDefinition>
ShaderModuleImpl::descriptorBindingDefinitions() const
{
	return mDescriptorBindings;
}


VkShaderModule
ShaderModuleImpl::getVkShaderModule()
{
	return mShaderModule;
}
