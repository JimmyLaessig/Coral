#include <Coral/Vulkan/ShaderModuleImpl.hpp>

#include <Coral/Finally.hpp>

#include <spirv_reflect.h>

#include <algorithm>
#include <assert.h>
#include <optional>
#include <span>
#include <string>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

std::optional<CoAttributeFormat>
convert(SpvReflectFormat format)
{
    switch (format)
    {
        case SPV_REFLECT_FORMAT_R16_UINT:            return CO_ATTRIBUTE_FORMAT_UINT16;
        case SPV_REFLECT_FORMAT_R32_UINT:            return CO_ATTRIBUTE_FORMAT_UINT32;
        case SPV_REFLECT_FORMAT_R16_SINT:            return CO_ATTRIBUTE_FORMAT_INT16;
        case SPV_REFLECT_FORMAT_R32_SINT:            return CO_ATTRIBUTE_FORMAT_INT32;
        case SPV_REFLECT_FORMAT_R32_SFLOAT:          return CO_ATTRIBUTE_FORMAT_FLOAT;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:       return CO_ATTRIBUTE_FORMAT_VEC2F;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:    return CO_ATTRIBUTE_FORMAT_VEC3F;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return CO_ATTRIBUTE_FORMAT_VEC4F;
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

    constexpr auto BoolFlags        = SPV_REFLECT_TYPE_FLAG_BOOL;
    constexpr auto IntFlags         = SPV_REFLECT_TYPE_FLAG_INT;
    constexpr auto FloatFlags       = SPV_REFLECT_TYPE_FLAG_FLOAT;
    constexpr auto MatrixFlags      = SPV_REFLECT_TYPE_FLAG_MATRIX | SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_FLOAT;
    constexpr auto FloatVectorFlags = SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_FLOAT;
    constexpr auto IntVectorFlags   = SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_INT;
    constexpr auto StructFlags      = SPV_REFLECT_TYPE_FLAG_STRUCT | SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK;
    switch (variable.type_description->type_flags)
    {
    case BoolFlags:
        result.members.push_back({ CO_UNIFORM_FORMAT_BOOL, fullName , 1, variable.size, variable.padded_size });
        break;
    case IntFlags:
        result.members.push_back({ CO_UNIFORM_FORMAT_INT32, fullName , 1, variable.size, variable.padded_size });
        break;
    case FloatFlags:
        result.members.push_back({ CO_UNIFORM_FORMAT_FLOAT, fullName , 1, variable.size, variable.padded_size });
        break;
    case FloatVectorFlags:
        switch (traits.numeric.vector.component_count)
        {
        case 2:
            result.members.push_back({ CO_UNIFORM_FORMAT_VEC2F, fullName , 1, variable.size, variable.padded_size });
            break;
        case 3:
            result.members.push_back({ CO_UNIFORM_FORMAT_VEC3F, fullName , 1, variable.size, variable.padded_size });
            break;
        case 4:
            result.members.push_back({ CO_UNIFORM_FORMAT_VEC4F, fullName , 1, variable.size, variable.padded_size });
            break;
        default:
            assert(false);
        }
        break;
    case IntVectorFlags:
        switch (traits.numeric.vector.component_count)
        {
        case 2:
            result.members.push_back({ CO_UNIFORM_FORMAT_VEC2I, fullName , 1, variable.size, variable.padded_size });
            break;
        case 3:
            result.members.push_back({ CO_UNIFORM_FORMAT_VEC3I, fullName , 1, variable.size, variable.padded_size });
            break;
        case 4:
            result.members.push_back({ CO_UNIFORM_FORMAT_VEC4I, fullName , 1, variable.size, variable.padded_size });
            break;
        default:
            assert(false);
        }
        break;
    case MatrixFlags:
        if (traits.numeric.matrix.column_count == 4 && traits.numeric.matrix.row_count == 4)
        {
            result.members.push_back({ CO_UNIFORM_FORMAT_MAT44F, fullName , 1, variable.size, variable.padded_size });
        }
        else if (traits.numeric.matrix.column_count == 3 && traits.numeric.matrix.row_count == 3)
        {
            result.members.push_back({ CO_UNIFORM_FORMAT_MAT33F, fullName , 1, variable.size, variable.padded_size });
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

} // namespace


ShaderModuleImpl::~ShaderModuleImpl()
{
    if (mShaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(context().getVkDevice(), mShaderModule, nullptr);
    }
}


std::optional<Coral::ShaderModule::CreateError>
ShaderModuleImpl::init(const ShaderModule::CreateConfig& config)
{
    mName        = config.name;
    mShaderStage = config.stage;
    mEntryPoint  = config.entryPoint;

    VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.pCode    = (uint32_t*)config.source.data();
    createInfo.codeSize = config.source.size();

    if (vkCreateShaderModule(context().getVkDevice(), &createInfo, nullptr, &mShaderModule) != VK_SUCCESS)
    {
        return ShaderModule::CreateError::INTERNAL_ERROR;
    }

    if (!reflect(config.source))
    {
        return ShaderModule::CreateError::INTERNAL_ERROR;
    }

    return {};
}


bool
ShaderModuleImpl::reflect(std::span<const std::byte> spirvCode)
{
    SpvReflectShaderModule module{};

    if (spvReflectCreateShaderModule(spirvCode.size_bytes(), spirvCode.data(), &module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        return false;
    }

    Finally cleanup([&]() { spvReflectDestroyShaderModule(&module); });

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

    if (sets.size() > 1 || sets.size() == 1 && sets.front()->set !=  0)
    {
        return false;
    }

    for (auto set : sets)
    {
        for (auto binding : std::span{ set->bindings, set->binding_count })
        {    
            auto& descriptorBinding      = mDescriptorLayout.emplace_back();
            //descriptorBinding.set      = set->set;
            descriptorBinding.binding = binding->binding;

            switch (binding->descriptor_type)
            {
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    descriptorBinding.name       = binding->type_description->type_name;
                    descriptorBinding.definition = createUniformBlockBinding(*binding);
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    descriptorBinding.name       = binding->name;
                    descriptorBinding.definition = TextureDefinition{};
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                    descriptorBinding.name       = binding->name;
                    descriptorBinding.definition = SamplerDefinition{};
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    descriptorBinding.name       = binding->name;
                    descriptorBinding.definition = Coral::CombinedTextureSamplerDefinition{};
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

        auto& description    = mInputAttributeLayout.emplace_back();
        description.location = variable->location;
        description.name     = variable->name;
        description.format   = *format;
    }

    std::sort(mInputAttributeLayout.begin(), mInputAttributeLayout.end(), 
              [](const auto& lhs, const auto& rhs) { return lhs.location < rhs.location; });

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

        auto& description    = mOutputAttributeLayout.emplace_back();
        description.location = variable->location;
        description.name     = variable->name;
        description.format   = *format;
    }

    return true;
}


const std::string&
ShaderModuleImpl::name() const
{
    return mName;
}


CoShaderStage
ShaderModuleImpl::shaderStage() const
{
    return mShaderStage;
}


const std::string&
ShaderModuleImpl::entryPoint() const
{
    return mEntryPoint;
}


const Coral::AttributeLayout&
ShaderModuleImpl::inputAttributeLayout() const
{
    return mInputAttributeLayout;
}


const Coral::AttributeLayout&
ShaderModuleImpl::outputAttributeLayout() const
{
    return mOutputAttributeLayout;
}


const Coral::DescriptorLayout&
ShaderModuleImpl::descriptorLayout() const
{
    return mDescriptorLayout;
}


VkShaderModule
ShaderModuleImpl::getVkShaderModule()
{
    return mShaderModule;
}
