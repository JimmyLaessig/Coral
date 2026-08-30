#include "ShaderModuleImpl.hpp"

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

void
destroy(const CoStructMemberInfo& memberInfo);


void
destroy(const CoStructInfo& structInfo)
{
    delete structInfo.pTypeName;

    for (auto& member : std::span(structInfo.pMembers, structInfo.memberCount))
    {
        destroy(member);
    }

    delete[] structInfo.pMembers;
}


void
destroy(const CoStructMemberInfo& memberInfo)
{
    delete(memberInfo.pName);

    switch (memberInfo.type)
    {
        case CO_STRUCT_MEMBER_TYPE_SCALAR:
            break;
        case CO_STRUCT_MEMBER_TYPE_VECTOR:
            break;
        case CO_STRUCT_MEMBER_TYPE_MATRIX:
            break;
        case CO_STRUCT_MEMBER_TYPE_STRUCT:
            destroy(memberInfo.structure);
            break;
    }
}


void
destroy(const CoBufferInfo& bufferInfo)
{
    destroy(bufferInfo.structure);
}


void
destroy(const CoDescriptorInfo& info)
{
    delete info.pName;

    switch (info.type)
    {
    case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        destroy(info.buffer);
        break;
    case CO_DESCRIPTOR_TYPE_IMAGE:
        break;
    case CO_DESCRIPTOR_TYPE_SAMPLER:
        break;
    case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        break;
    }
}


void
destroy(const CoAttributeBindingInfo& attr)
{
    delete attr.pName;
}


void
destroy(CoDescriptorLayout& layout)
{
    for (auto& descriptor : std::span(layout.pDescriptorInfos, layout.descriptorInfosCount))
    {
        destroy(descriptor);
    }
    delete[] layout.pDescriptorInfos;
}


void
destroy(CoAttributeLayout& layout)
{
    for (auto& attr : std::span(layout.pInputAttributeBindingInfos, layout.inputAttributeBindingInfoCount))
    {
        destroy(attr);
    }
    delete[] layout.pInputAttributeBindingInfos;

    for (auto& attr : std::span(layout.pOutputAttributeBindingInfos, layout.outputAttributeBindingInfoCount))
    {
        destroy(attr);
    }
    delete[] layout.pOutputAttributeBindingInfos;
}


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


const char*
copy(const char* src)
{
    if (src == nullptr)
    {
        return nullptr;
    }
    auto length = std::strlen(src);
    if (length == 0)
    {
        return nullptr;
    }
    char* copy = new char[length + 1];
    std::strcpy(copy, src);

    return copy;
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
                case 1:  return CO_SCALAR_TYPE_INT8;
                case 2:  return CO_SCALAR_TYPE_INT16;
                case 4:  return CO_SCALAR_TYPE_INT32;
                case 8:  return CO_SCALAR_TYPE_INT64;
                default: assert(false);
            }
        }
        else
        {
            switch (size)
            {
                case 1:  return CO_SCALAR_TYPE_UINT8;
                case 2:  return CO_SCALAR_TYPE_UINT16;
                case 4:  return CO_SCALAR_TYPE_UINT32;
                case 8:  return CO_SCALAR_TYPE_UINT64;
                default: assert(false);
            }
        }
    }
    // TODO Parse bool
    return CO_SCALAR_TYPE_INT32;
}


bool
reflect(const SpvReflectBlockVariable& variable, CoStructMemberInfo& member)
{
    member.pName     = copy(variable.name);
    member.offset    = variable.offset;
    member.stride    = variable.padded_size;
    member.count     = 1;
    member.typeFlags = CO_STRUCT_MEMBER_TYPE_FLAG_NONE_BIT;
    auto& traits     = variable.type_description->traits;

    if (SPV_REFLECT_TYPE_FLAG_REF & variable.type_description->type_flags)
    {
        member.typeFlags |= CO_STRUCT_MEMBER_TYPE_FLAG_POINTER_BIT;
    }
    if (SPV_REFLECT_TYPE_FLAG_STRUCT & variable.type_description->type_flags)
    {
        member.type                  = CO_STRUCT_MEMBER_TYPE_STRUCT;
        member.structure.memberCount = variable.member_count;
        auto members                 = new CoStructMemberInfo[member.structure.memberCount];
        member.structure.pMembers    = members;

        for (size_t i = 0; i < member.structure.memberCount; ++i)
        {
            if (!reflect(variable.members[i], members[i]))
            {
                return false;
            }
        }
        return true;
    }
    if (SPV_REFLECT_TYPE_FLAG_MATRIX & variable.type_description->type_flags)
    {
        member.type               = CO_STRUCT_MEMBER_TYPE_MATRIX;
        member.matrix.rowCount    = traits.numeric.matrix.row_count;
        member.matrix.columnCount = traits.numeric.matrix.column_count;
        member.matrix.stride      = traits.numeric.matrix.stride;
        auto elementCount         = member.matrix.rowCount * member.matrix.columnCount;
        member.matrix.dataType    = ::toScalarType(*variable.type_description);
        member.matrix.layout      = (variable.decoration_flags & SPV_REFLECT_DECORATION_ROW_MAJOR) ? CO_MATRIX_LAYOUT_ROW_MAJOR 
                                                                                                   : CO_MATRIX_LAYOUT_COLUMN_MAJOR;
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_VECTOR & variable.type_description->type_flags)
    {
        member.type                  = CO_STRUCT_MEMBER_TYPE_VECTOR;
        member.vector.componentCount = traits.numeric.vector.component_count;
        member.vector.dataType       = ::toScalarType(*variable.type_description);
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_BOOL & variable.type_description->type_flags)
    {
        member.type            = CO_STRUCT_MEMBER_TYPE_SCALAR;
        member.scalar.dataType = CO_SCALAR_TYPE_BOOL;
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_INT & variable.type_description->type_flags)
    {
        member.type            = CO_STRUCT_MEMBER_TYPE_SCALAR;
        member.scalar.dataType = ::toScalarType(*variable.type_description);
        return true;
    }

    if (SPV_REFLECT_TYPE_FLAG_FLOAT & variable.type_description->type_flags)
    {
        member.type            = CO_STRUCT_MEMBER_TYPE_SCALAR;
        member.scalar.dataType = ::toScalarType(*variable.type_description);
        return true;
    }

    return false;
}


bool
reflect(const SpvReflectBlockVariable& block, CoBufferInfo& buffer)
{
    return reflect(block, buffer.structure);
}


bool
reflect(const SpvReflectDescriptorBinding& binding, CoDescriptorInfo& info)
{
    info.binding = binding.binding;
    info.pName   = copy(binding.name);
    switch (binding.descriptor_type)
    {
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            info.type = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            return reflect(binding.block, info.buffer);
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            info.type = CO_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            return reflect(binding.block, info.buffer);
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            info.type  = CO_DESCRIPTOR_TYPE_IMAGE;
            return true;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
            info.type  = CO_DESCRIPTOR_TYPE_SAMPLER;
            return true;
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            info.type  = CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            return true;
      
        default:
            assert(false);
    }

    std::unreachable();
}


bool
reflect(const SpvReflectInterfaceVariable& variable, CoAttributeBindingInfo& info)
{
    auto format = convert(variable.format);
    if (!format)
    {
        // TODO Log unsupported input format
        return false;
    }
    
    info.format   = *format;
    info.location = variable.location;
    info.pName    = copy(variable.name);
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
    mName        = config.pName ? config.pName : "";
    mShaderStage = config.stage;
    mEntryPoint  = config.pEntryPoint ?  config.pEntryPoint : "";

    std::span<const uint32_t> spirvCode(reinterpret_cast<const uint32_t*>(config.pSource), config.sourceCount / sizeof(uint32_t));

    VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.pCode    = spirvCode.data();
    createInfo.codeSize = config.sourceCount;

    if (vkCreateShaderModule(context().getVkDevice(), &createInfo, nullptr, &mShaderModule) != VK_SUCCESS)
    {
        return ShaderModule::CreateError::INTERNAL_ERROR;
    }

    if (!reflect(spirvCode))
    {
        return ShaderModule::CreateError::INTERNAL_ERROR;
    }

    return {};
}


bool
ShaderModuleImpl::reflect(std::span<const uint32_t> spirvCode)
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

    if (!(sets.size() == 1 && sets.front()->set == 0))
    {
        return false;
    }

    auto set = sets.front();
    mDescriptorLayout.descriptorInfosCount = set->binding_count;
    auto descriptorInfos                   = mDescriptorLayout.descriptorInfosCount ? new CoDescriptorInfo[mDescriptorLayout.descriptorInfosCount] : nullptr;
    mDescriptorLayout.pDescriptorInfos     = descriptorInfos;

    for (auto [a, b] : std::views::zip(std::span(set->bindings, set->binding_count),
                                       std::span(descriptorInfos, mDescriptorLayout.descriptorInfosCount)))
    {
        if (!::reflect(*a, b))
        {
            return false;
        }
    }

    std::vector<SpvReflectInterfaceVariable*> inputVariables;
    {
        uint32_t count{ 0 };
        spvReflectEnumerateInputVariables(&module, &count, nullptr);
        inputVariables.resize(count);
        spvReflectEnumerateInputVariables(&module, &count, inputVariables.data());
    }

    std::erase_if(inputVariables, [](const SpvReflectInterfaceVariable* v) 
    { 
        return v->built_in != -1;
    });

    mAttributeLayout.inputAttributeBindingInfoCount = static_cast<uint32_t>(inputVariables.size());
    auto inputAttributeBindingInfos                 = mAttributeLayout.inputAttributeBindingInfoCount ? new CoAttributeBindingInfo[mAttributeLayout.inputAttributeBindingInfoCount] : nullptr;
    mAttributeLayout.pInputAttributeBindingInfos    = inputAttributeBindingInfos;

    for (auto [a, b] : std::views::zip(inputVariables,
                                       std::span(inputAttributeBindingInfos,
                                                 mAttributeLayout.inputAttributeBindingInfoCount)))
    {
        if (!::reflect(*a, b))
        {
            return false;
        }
    }

    std::vector<SpvReflectInterfaceVariable*> outputVariables;
    {
        uint32_t count{ 0 };
        spvReflectEnumerateOutputVariables(&module, &count, nullptr);
        outputVariables.resize(count);
        spvReflectEnumerateOutputVariables(&module, &count, outputVariables.data());
    }
    mAttributeLayout.outputAttributeBindingInfoCount = static_cast<uint32_t>(outputVariables.size());
    auto outputAttributeBindingInfos                 = mAttributeLayout.outputAttributeBindingInfoCount ? new CoAttributeBindingInfo[mAttributeLayout.outputAttributeBindingInfoCount] : nullptr;
    mAttributeLayout.pOutputAttributeBindingInfos    = outputAttributeBindingInfos;

    for (auto [a, b] : std::views::zip(inputVariables,
                                       std::span(outputAttributeBindingInfos, mAttributeLayout.outputAttributeBindingInfoCount)))
    {
        if (!::reflect(*a, b))
        {
            return false;
        }
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


VkShaderModule
ShaderModuleImpl::getVkShaderModule()
{
    return mShaderModule;
}


const CoDescriptorLayout&
ShaderModuleImpl::descriptorLayout() const
{
    return mDescriptorLayout;
}


const CoAttributeLayout&
ShaderModuleImpl::attributeLayout() const
{
    return mAttributeLayout;
}
