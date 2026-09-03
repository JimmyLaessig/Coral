
#include "DescriptorSetImpl.hpp"

#include "Visitor.hpp"
#include "ShaderModuleImpl.hpp"
#include "VulkanFormat.hpp"

#include "BufferImpl.hpp"
#include "SamplerImpl.hpp"
#include "ImageImpl.hpp"

#include <cassert>
#include <vector>
#include <span>
#include <variant>


using namespace Coral::Vulkan;

namespace
{

VkDescriptorType convert(CoDescriptorType type)
{
    switch (type)
    {
    case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case CO_DESCRIPTOR_TYPE_IMAGE:                  return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case CO_DESCRIPTOR_TYPE_SAMPLER:                return VK_DESCRIPTOR_TYPE_SAMPLER;
    case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    std::unreachable();
}

} // namespace

DescriptorSetImpl::~DescriptorSetImpl()
{
    if (mLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(context().getVkDevice(), mLayout, nullptr);
    }

    if (mDescriptorSet != VK_NULL_HANDLE)
    {
        context().getDescriptorPool().freeDescriptorSet(mDescriptorSet);
    }
}

std::optional<Coral::DescriptorSet::CreateError>
DescriptorSetImpl::init(const DescriptorSet::CreateConfig& config)
{
    auto device = context().getVkDevice();

    auto inputBindings = std::span(config.pDescriptorBindings, config.descriptorBindingCount);

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& binding: inputBindings)
    {
        bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = binding.binding,
            .descriptorType     = ::convert(binding.type),
            .descriptorCount    = 1,
            .stageFlags         = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = VK_NULL_HANDLE,
        });
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    layoutInfo.pBindings    = bindings.data();
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    
    auto& pool = context().getDescriptorPool();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mLayout) != VK_SUCCESS)
    {
        return Coral::DescriptorSet::CreateError::INTERNAL_ERROR;
    }

    mDescriptorSet = pool.allocateDescriptorSet(mLayout);

    if (mDescriptorSet == VK_NULL_HANDLE)
    {
        return Coral::DescriptorSet::CreateError::INTERNAL_ERROR;
    }



    for (const auto& binding : inputBindings)
    {
        VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        descriptorWrite.dstSet          = mDescriptorSet;
        descriptorWrite.dstBinding      = binding.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = ::convert(binding.type);

        switch (binding.type)
        {
        case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        {
            auto impl = std::static_pointer_cast<BufferImpl>(binding.buffer->impl);

            std::vector<VkDescriptorBufferInfo> bufferInfos;
            bufferInfos.push_back({
                .buffer = impl->getVkBuffer(),
                .offset = 0,
                .range  = VK_WHOLE_SIZE,
            });

            descriptorWrite.pBufferInfo     = bufferInfos.data();
            descriptorWrite.descriptorCount = static_cast<uint32_t>(bufferInfos.size());

            vkUpdateDescriptorSets(device,
                                   1,
                                   &descriptorWrite,
                                   /*descriptorCopyCount*/ 0,
                                   /*pDescriptorCopies*/ nullptr);
            break;
        }
        case CO_DESCRIPTOR_TYPE_IMAGE:
        {
            auto impl = std::static_pointer_cast<ImageImpl>(binding.image->impl);

            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.push_back({
                .sampler     = VK_NULL_HANDLE,
                .imageView   = impl->getVkImageView(),
                .imageLayout = impl->getPreferredImageLayout(),
            });

            descriptorWrite.pImageInfo      = imageInfos.data();
            descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());

            vkUpdateDescriptorSets(device,
                                   1,
                                   &descriptorWrite,
                                   /*descriptorCopyCount*/ 0,
                                   /*pDescriptorCopies*/ nullptr);
            break;
        }
        case CO_DESCRIPTOR_TYPE_SAMPLER:
        {
            auto impl = std::static_pointer_cast<SamplerImpl>(binding.sampler->impl);

            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.push_back({
                .sampler   = impl->getVkSampler(),
                .imageView = VK_NULL_HANDLE,
            });

            descriptorWrite.pImageInfo      = imageInfos.data();
            descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());

            vkUpdateDescriptorSets(device,
                                   1,
                                   &descriptorWrite,
                                   /*descriptorCopyCount*/ 0,
                                   /*pDescriptorCopies*/ nullptr);
            break;
        }
        case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        {
            auto imageImpl   = std::static_pointer_cast<ImageImpl>(binding.combinedImageSampler.image->impl);
            auto samplerImpl = std::static_pointer_cast<SamplerImpl>(binding.combinedImageSampler.sampler->impl);

            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.push_back({
                .sampler     = samplerImpl->getVkSampler(),
                .imageView   = imageImpl->getVkImageView(),
                .imageLayout = imageImpl->getPreferredImageLayout(),
            });

            descriptorWrite.pImageInfo      = imageInfos.data();
            descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());

            vkUpdateDescriptorSets(device,
                                   1,
                                   &descriptorWrite,
                                   /*descriptorCopyCount*/ 0,
                                   /*pDescriptorCopies*/ nullptr);
            break;
        }
        }
    }

    return {};
}