#include "ShaderModuleReflection.hpp"

#include "ShaderModule.hpp"
#include "Visitor.hpp"

#include <cassert>
#include <ranges>
#include <span>

namespace
{

void
destroy(const CoReflectDescriptor& descriptor)
{
    delete[] descriptor.pName;
}


void
destroy(const CoReflectDescriptorSet& descriptorSet)
{
    for (const auto& descriptor : std::span(descriptorSet.pDescriptors, descriptorSet.descriptorCount))
    {
        destroy(descriptor);
    }

    delete[] descriptorSet.pDescriptors;
}


void
destroy(const CoReflectAttribute& attribute)
{
    delete[] attribute.pName;
}


CoStructureInfo
create(const Coral::StructureInfo& source);


CoStructureMemberInfo
create(const Coral::StructureMember& source)
{
    CoStructureMemberInfo target{};
    target.offset    = source.offset;
    target.typeFlags = source.typeFlags;
    auto name        = new char[source.name.size() + 1];
    target.pName     = name;
    std::memcpy(name, source.name.c_str(), source.name.size() + 1);

    std::visit(Coral::Visitor
    {
        [&](const Coral::ScalarInfo& info)
        {
            target.type   = CO_STRUCTURE_MEMBER_TYPE_SCALAR;
            target.scalar = { info.dataType };
        },
        [&](const Coral::VectorInfo& info)
        {
            target.type   = CO_STRUCTURE_MEMBER_TYPE_VECTOR;
            target.vector = { info.dataType, info.components };
        },
        [&](const Coral::MatrixInfo& info)
        {
            target.type   = CO_STRUCTURE_MEMBER_TYPE_MATRIX;
            target.matrix = { info.dataType, info.rows, info.columns, info.stride, info.layout };
        },
        [&](const Coral::StructureInfo& info)
        {
            target.type      = CO_STRUCTURE_MEMBER_TYPE_STRUCTURE;
            target.structure = create(info);
        }
    }, source.typeInfo);

    return target;
}


CoStructureInfo
create(const Coral::StructureInfo& source)
{
    CoStructureInfo target{};
    target.memberCount = static_cast<uint32_t>(source.members.size());
    target.size        = source.size;
    auto typeName      = new char[source.typeName.size() + 1];
    target.pTypeName   = typeName;
    std::memcpy(typeName, source.typeName.c_str(), source.typeName.size() + 1);

    if (target.memberCount > 0)
    {
        auto members    = new CoStructureMemberInfo[target.memberCount];
        target.pMembers = members;
        for (const auto& [i, member] : std::views::enumerate(source.members))
        {
            members[i] = create(member);
        }
    }
    else
    {
        target.pMembers = nullptr;
    }

    return target;
}


CoBufferInfo
create(const Coral::BufferInfo& source)
{
    CoBufferInfo target{};
    target.structure = create(source.structure);
    return target;
}


CoReflectDescriptor
create(const Coral::ReflectDescriptor& source)
{
    CoReflectDescriptor target{};
    target.binding = source.binding;
    auto name      = new char[source.name.size() + 1];
    target.pName   = name;
    std::memcpy(name, source.name.c_str(), source.name.size() + 1);

    std::visit(Coral::Visitor{
        [&](const Coral::BufferInfo& t)
        {
            target.type = t.bufferType == CO_BUFFER_TYPE_UNIFORM ? CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER 
                                                                 : CO_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            target.buffer = create(t);
        },
        [&](const Coral::ImageInfo& t) 
        {
            target.type = CO_DESCRIPTOR_TYPE_IMAGE;
        },
        [&](const Coral::SamplerInfo& t) 
        {
            target.type = CO_DESCRIPTOR_TYPE_SAMPLER;
        },
        [&](const Coral::CombinedImageSamplerInfo& t)
        {
            target.type = CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
    }, source.info);

    return target;
}


CoReflectDescriptorSet
create(const Coral::ReflectDescriptorSet& source)
{
    CoReflectDescriptorSet target{};
    target.set             = source.set;
    target.descriptorCount = static_cast<uint32_t>(source.descriptors.size());

    if (target.descriptorCount > 0)
    {
        auto descriptors    = new CoReflectDescriptor[target.descriptorCount];
        target.pDescriptors = descriptors;

        for (const auto& [i, descriptor] : std::views::enumerate(source.descriptors))
        {
            descriptors[i] = create(descriptor);
        }
    }
    else
    {
        target.pDescriptors = nullptr;
    }

    return target;
}

CoReflectAttribute
create(const Coral::ReflectAttribute& source)
{
    auto name = new char[source.name.size() + 1];
    std::memcpy(name, source.name.c_str(), source.name.size() + 1);
    return 
    {
        .pName    = name,
        .location = source.location,
        .format   = source.format
    };
}

} // namespace

namespace Coral
{

void
destroy(const CoShaderModuleReflectionInfo& info)
{
    if (info.pDescriptorSets)
    {
        for (const auto& descriptorSet : std::span(info.pDescriptorSets, 
                                                   info.descriptorSetCount))
        {
            ::destroy(descriptorSet);
        }
        delete[] info.pDescriptorSets;
    }

    if (info.pInputAttributes)
    {
        for (const auto& attribute : std::span(info.pInputAttributes, 
                                               info.inputAttributeCount))
        {
            ::destroy(attribute);
        }
        delete[] info.pInputAttributes;
    }

    if (info.pOutputAttributes)
    {
        for (const auto& attribute : std::span(info.pOutputAttributes, 
                                               info.outputAttributeCount))
        {
            ::destroy(attribute);
        }
        delete[] info.pOutputAttributes;
    }
}


CoShaderModuleReflectionInfo
create(const ShaderModuleReflectionInfo& info)
{
    CoShaderModuleReflectionInfo result{};
    result.descriptorSetCount = static_cast<uint32_t>(info.descriptorSets.size());
    if (result.descriptorSetCount > 0)
    {
        auto sets = new CoReflectDescriptorSet[info.descriptorSets.size()];
        result.pDescriptorSets = sets;
        for (const auto& [i, descriptorSet] : std::views::enumerate(info.descriptorSets))
        {
            sets[i] = ::create(descriptorSet);
        }
    }
    else
    {
        result.pDescriptorSets = nullptr;
    }

    result.inputAttributeCount = static_cast<uint32_t>(info.inputAttributes.size());
    if (result.inputAttributeCount > 0)
    {
        auto attributes = new CoReflectAttribute[info.inputAttributes.size()];
        result.pInputAttributes = attributes;
        for (const auto& [i, attribute] : std::views::enumerate(info.inputAttributes))
        {
            attributes[i] = ::create(attribute);
        }
    }
    else
    {
        result.pInputAttributes = nullptr;
    }

    result.outputAttributeCount = static_cast<uint32_t>(info.outputAttributes.size());
    if (result.outputAttributeCount > 0)
    {
        auto attributes = new CoReflectAttribute[info.outputAttributes.size()];
        result.pOutputAttributes = attributes;
        for (const auto& [i, attribute] : std::views::enumerate(info.outputAttributes))
        {
            attributes[i] = ::create(attribute);
        }
    }
    else
    {
        result.pOutputAttributes = nullptr;
    }

    return result;
}

} // namespace Coral


uint32_t
coScalarTypeGetSizeInBytes(const CoScalarType scalarType)
{
    switch (scalarType)
    {
        case CO_SCALAR_TYPE_BOOL:    return sizeof(bool);
        case CO_SCALAR_TYPE_FLOAT16: return sizeof(uint16_t);
        case CO_SCALAR_TYPE_FLOAT32: return sizeof(float);
        case CO_SCALAR_TYPE_FLOAT64: return sizeof(double);
        case CO_SCALAR_TYPE_INT8:    return sizeof(int8_t);
        case CO_SCALAR_TYPE_INT16:   return sizeof(int16_t);
        case CO_SCALAR_TYPE_INT32:   return sizeof(int32_t);
        case CO_SCALAR_TYPE_INT64:   return sizeof(int64_t);
        case CO_SCALAR_TYPE_UINT8:   return sizeof(uint8_t);
        case CO_SCALAR_TYPE_UINT16:  return sizeof(uint16_t);
        case CO_SCALAR_TYPE_UINT32:  return sizeof(uint32_t);
        case CO_SCALAR_TYPE_UINT64:  return sizeof(uint64_t);
    }

    assert(false);
    return 0;
}

void
coShaderModuleReflect(const CoShaderModule shaderModule, CoShaderModuleReflectionInfo* pShaderModuleReflectionInfo)
{
    if (!shaderModule->mReflection)
    {
        shaderModule->mReflection = Coral::create(shaderModule->impl->getReflectionInfo());
    }

    *pShaderModuleReflectionInfo = *shaderModule->mReflection;
}
