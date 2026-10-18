#include "ShaderModuleImpl.hpp"

#include "ContextImpl.hpp"
#include "Finally.hpp"

#include <spirv_reflect.h>

#include <assert.h>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <ranges>

using namespace Coral::Vulkan;

namespace
{

std::optional<CoAttributeFormat>
convert(SpvReflectFormat format)
{
    switch (format)
    {
        case SPV_REFLECT_FORMAT_UNDEFINED:           return std::nullopt;
        case SPV_REFLECT_FORMAT_R16_UINT:            return CO_ATTRIBUTE_FORMAT_R_U16;
        case SPV_REFLECT_FORMAT_R16_SINT:            return CO_ATTRIBUTE_FORMAT_R_I16;
        case SPV_REFLECT_FORMAT_R16_SFLOAT:          return CO_ATTRIBUTE_FORMAT_R_F16;
        case SPV_REFLECT_FORMAT_R16G16_UINT:         return CO_ATTRIBUTE_FORMAT_RG_U16;
        case SPV_REFLECT_FORMAT_R16G16_SINT:         return CO_ATTRIBUTE_FORMAT_RG_I16;
        case SPV_REFLECT_FORMAT_R16G16_SFLOAT:       return CO_ATTRIBUTE_FORMAT_RG_F16;
        case SPV_REFLECT_FORMAT_R16G16B16_UINT:      return CO_ATTRIBUTE_FORMAT_RGB_U16;
        case SPV_REFLECT_FORMAT_R16G16B16_SINT:      return CO_ATTRIBUTE_FORMAT_RGB_I16;
        case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:    return CO_ATTRIBUTE_FORMAT_RGB_F16;
        case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:   return CO_ATTRIBUTE_FORMAT_RGBA_U16;
        case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:   return CO_ATTRIBUTE_FORMAT_RGBA_I16;
        case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: return CO_ATTRIBUTE_FORMAT_RGBA_F16;
        case SPV_REFLECT_FORMAT_R32_UINT:            return CO_ATTRIBUTE_FORMAT_R_U32;
        case SPV_REFLECT_FORMAT_R32_SINT:            return CO_ATTRIBUTE_FORMAT_R_I32;
        case SPV_REFLECT_FORMAT_R32_SFLOAT:          return CO_ATTRIBUTE_FORMAT_R_F32;
        case SPV_REFLECT_FORMAT_R32G32_UINT:         return CO_ATTRIBUTE_FORMAT_RG_U32;
        case SPV_REFLECT_FORMAT_R32G32_SINT:         return CO_ATTRIBUTE_FORMAT_RG_I32;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:       return CO_ATTRIBUTE_FORMAT_RG_F32;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:      return CO_ATTRIBUTE_FORMAT_RGB_U32;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:      return CO_ATTRIBUTE_FORMAT_RGB_I32;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:    return CO_ATTRIBUTE_FORMAT_RGB_F32;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:   return CO_ATTRIBUTE_FORMAT_RGBA_U32;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:   return CO_ATTRIBUTE_FORMAT_RGBA_I32;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return CO_ATTRIBUTE_FORMAT_RGBA_F32;
        case SPV_REFLECT_FORMAT_R64_UINT:            return CO_ATTRIBUTE_FORMAT_R_U64;
        case SPV_REFLECT_FORMAT_R64_SINT:            return CO_ATTRIBUTE_FORMAT_R_I64;
        case SPV_REFLECT_FORMAT_R64_SFLOAT:          return CO_ATTRIBUTE_FORMAT_R_F64;
        case SPV_REFLECT_FORMAT_R64G64_UINT:         return CO_ATTRIBUTE_FORMAT_RG_U64;
        case SPV_REFLECT_FORMAT_R64G64_SINT:         return CO_ATTRIBUTE_FORMAT_RG_I64;
        case SPV_REFLECT_FORMAT_R64G64_SFLOAT:       return CO_ATTRIBUTE_FORMAT_RG_F64;
        case SPV_REFLECT_FORMAT_R64G64B64_UINT:      return CO_ATTRIBUTE_FORMAT_RGB_U64;
        case SPV_REFLECT_FORMAT_R64G64B64_SINT:      return CO_ATTRIBUTE_FORMAT_RGB_I64;
        case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:    return CO_ATTRIBUTE_FORMAT_RGB_F64;
        case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:   return CO_ATTRIBUTE_FORMAT_RGBA_U64;
        case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:   return CO_ATTRIBUTE_FORMAT_RGBA_I64;
        case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return CO_ATTRIBUTE_FORMAT_RGBA_F64;
    }

    std::unreachable();
}


CoScalarType
toScalarType(const SpvReflectTypeDescription& desc)
{
    auto size = desc.traits.numeric.scalar.width / 8;
    if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
    {
        switch (size)
        {
            case 4:  return CO_SCALAR_TYPE_FLOAT32;
            case 8:  return CO_SCALAR_TYPE_FLOAT64;
            default: assert(false);
        }
    }
    if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_INT)
    {
        if (desc.traits.numeric.scalar.signedness == 1)
        {
            switch (size)
            {
                case 1: return CO_SCALAR_TYPE_INT8;
                case 2: return CO_SCALAR_TYPE_INT16;
                case 4: return CO_SCALAR_TYPE_INT32;
                case 8: return CO_SCALAR_TYPE_INT64;
                default: assert(false);
            }
        }
        else
        {
            switch (size)
            {
                case 1: return CO_SCALAR_TYPE_UINT8;
                case 2: return CO_SCALAR_TYPE_UINT16;
                case 4: return CO_SCALAR_TYPE_UINT32;
                case 8: return CO_SCALAR_TYPE_UINT64;
                default: assert(false);
            }
        }
    }
    // TODO Parse bool
    return CO_SCALAR_TYPE_INT32;
}


bool
reflect(const SpvReflectBlockVariable& variable, Coral::StructureMember& member)
{
    member.name   = variable.name;
    member.offset = variable.offset;

    member.array.stride = variable.padded_size;
    member.array.dimensions.assign(variable.array.dims, variable.array.dims + variable.array.dims_count);

    auto& traits = variable.type_description->traits;
    if (SPV_REFLECT_TYPE_FLAG_REF & variable.type_description->type_flags)
    {
        member.typeFlags |= CO_STRUCTURE_MEMBER_TYPE_FLAG_POINTER_BIT;
    }
    if (SPV_REFLECT_TYPE_FLAG_STRUCT & variable.type_description->type_flags)
    {
        auto& structure    = member.typeInfo.emplace<Coral::StructureInfo>();
        structure.typeName = variable.type_description->type_name;
        structure.size     = variable.size;
        structure.members.resize(variable.member_count);
        for (const auto& [source, target] : std::views::zip(std::span(variable.members, variable.member_count), 
                                                            structure.members))
        {
            if (!reflect(source, target))
            {
                return false;
            }
        }
        return true;
    }
    if (SPV_REFLECT_TYPE_FLAG_MATRIX & variable.type_description->type_flags)
    {
        auto& matrix    = member.typeInfo.emplace<Coral::MatrixInfo>();
        matrix.rows     = traits.numeric.matrix.row_count;
        matrix.columns  = traits.numeric.matrix.column_count;
        matrix.stride   = traits.numeric.matrix.stride;
        matrix.dataType = ::toScalarType(*variable.type_description);
        matrix.layout   = (variable.decoration_flags & SPV_REFLECT_DECORATION_ROW_MAJOR) ? CO_MATRIX_LAYOUT_ROW_MAJOR 
                                                                                         : CO_MATRIX_LAYOUT_COLUMN_MAJOR;
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_VECTOR & variable.type_description->type_flags)
    {
        auto& vector      = member.typeInfo.emplace<Coral::VectorInfo>();
        vector.components = traits.numeric.vector.component_count;
        vector.dataType   = ::toScalarType(*variable.type_description);
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_BOOL & variable.type_description->type_flags)
    {
        auto& scalar    = member.typeInfo.emplace<Coral::ScalarInfo>();
        scalar.dataType = CO_SCALAR_TYPE_BOOL;
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_INT & variable.type_description->type_flags)
    {
        auto& scalar    = member.typeInfo.emplace<Coral::ScalarInfo>();
        scalar.dataType = ::toScalarType(*variable.type_description);
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_FLOAT & variable.type_description->type_flags)
    {
        auto& scalar     = member.typeInfo.emplace<Coral::ScalarInfo>();
        scalar.dataType = ::toScalarType(*variable.type_description);
        return true;
    }

    return false;
}


bool
reflect(const SpvReflectBlockVariable& block, Coral::BufferInfo& buffer)
{
    Coral::StructureMember member;
    if (!reflect(block, member))
    {
        return false;
    }

    auto structure = std::get_if<Coral::StructureInfo>(&member.typeInfo);
    if (!structure)
    {
        return false;
    }

    buffer.structure = *structure;
    return true;
}


bool
reflect(const SpvReflectDescriptorBinding& source, Coral::ReflectDescriptor& target)
{
    target.binding = source.binding;
    target.name    = source.name;

    switch (source.descriptor_type)
    {
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        {
            auto& info = target.info.emplace<Coral::BufferInfo>();
            info.bufferType = CO_BUFFER_TYPE_UNIFORM;
            return reflect(source.block, info);
        }
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        {
            auto& info = target.info.emplace<Coral::BufferInfo>();
            info.bufferType = CO_BUFFER_TYPE_STORAGE;
            return reflect(source.block, info);
        }
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            target.info.emplace<Coral::ImageInfo>();
            return true;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
            target.info.emplace<Coral::SamplerInfo>();
            return true;
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            target.info.emplace<Coral::CombinedImageSamplerInfo>();
            return true;

        default:
            assert(false);
            return false;
    }
}


bool
reflect(const SpvReflectInterfaceVariable& variable, Coral::ReflectAttribute& attribute)
{
    attribute.location = variable.location;
    attribute.name     = variable.name;

    if (auto format = ::convert(variable.format))
    {
        attribute.format = *format;
    }
    else
    {
        return false;
    }

    return true;
}


bool
reflect(const SpvReflectDescriptorSet& source, Coral::ReflectDescriptorSet& target)
{
    target.descriptors.resize(source.binding_count);
    for (size_t i = 0; i < source.binding_count; ++i)
    {
        if (!::reflect(*source.bindings[i], target.descriptors[i]))
        {
            return false;
        }
    }

    return true;
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
    auto code    = reinterpret_cast<const uint32_t*>(config.source.data());
    mSpirvCode   = std::vector<uint32_t>(code, code + config.source.size() /  sizeof(uint32_t));
    
    VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.pCode    = mSpirvCode.data();
    createInfo.codeSize = config.source.size();

    if (vkCreateShaderModule(context().getVkDevice(), &createInfo, nullptr, &mShaderModule) != VK_SUCCESS)
    {
        return ShaderModule::CreateError::INTERNAL_ERROR;
    }

    if (!reflect())
    {
        return ShaderModule::CreateError::INTERNAL_ERROR;
    }
    return {};
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


VkShaderModule
ShaderModuleImpl::getVkShaderModule()
{
    return mShaderModule;
}


const Coral::ShaderModuleReflectionInfo&
ShaderModuleImpl::getReflectionInfo() const
{
    return mReflection;
}


bool
ShaderModuleImpl::reflect()
{
    SpvReflectShaderModule module;
    if (spvReflectCreateShaderModule(mSpirvCode.size() * sizeof(uint32_t), mSpirvCode.data(), &module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        return {};
    }

    Finally cleanup([&]() { spvReflectDestroyShaderModule(&module); });

    //if (module.entry_point_name == nullptr)
    //{
    //    return false;
    //}
        
    std::vector<SpvReflectDescriptorSet*> sets;
    {
        uint32_t count{ 0 };
        spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
        sets.resize(count);
        spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
    }

    mReflection.descriptorSets.resize(sets.size());
    for (const auto& [spvSet, set] : std::views::zip(sets, mReflection.descriptorSets))
    {
        ::reflect(*spvSet, set);
    }

    auto reflectAttributes = [](std::vector<SpvReflectInterfaceVariable*> variables, std::vector<Coral::ReflectAttribute>& attributes)
    {
        std::erase_if(variables, [](const SpvReflectInterfaceVariable* v)
        {
            return v->built_in != -1;
        });

        attributes.resize(variables.size());

        for (const auto& [spvVar, attr] : std::views::zip(variables, attributes))
        {
            if (!::reflect(*spvVar, attr))
            {
                return false;
            }
        }

        return true;
    };

    std::vector<SpvReflectInterfaceVariable*> inputVariables;
    {
        uint32_t count{ 0 };
        spvReflectEnumerateInputVariables(&module, &count, nullptr);
        inputVariables.resize(count);
        spvReflectEnumerateInputVariables(&module, &count, inputVariables.data());
    }
        
    if (!reflectAttributes(std::move(inputVariables), mReflection.inputAttributes))
    {
        return false;
    }

    std::vector<SpvReflectInterfaceVariable*> outputVariables;
    {
        uint32_t count{ 0 };
        spvReflectEnumerateOutputVariables(&module, &count, nullptr);
        outputVariables.resize(count);
        spvReflectEnumerateOutputVariables(&module, &count, outputVariables.data());
    }

    if (!reflectAttributes(std::move(outputVariables), mReflection.outputAttributes))
    {
        return false;
    }
        
    return true;
}