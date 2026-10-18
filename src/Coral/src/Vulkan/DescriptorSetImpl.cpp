
#include "DescriptorSetImpl.hpp"

#include "BufferImpl.hpp"
#include "ContextImpl.hpp"
#include "ImageImpl.hpp"
#include "SamplerImpl.hpp"

#include <algorithm>
#include <vector>
#include <span>
#include "../Visitor.hpp"

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


DescriptorSetSize
countDescriptors(const std::vector<Coral::BindDescriptor>& bindings)
{
    DescriptorSetSize result{};

    for (const auto& binding : bindings)
    {
        std::visit(Coral::Visitor{
            [&](const Coral::BufferPtr& buffer)
            {
                result.storageBufferCount += buffer->type() == CO_BUFFER_TYPE_STORAGE;
                result.uniformBufferCount += buffer->type() == CO_BUFFER_TYPE_UNIFORM;
            },
            [&](const Coral::ImagePtr& image)
            {
                result.sampledImageCount++;
            },
            [&](const Coral::SamplerPtr& sampler)
            {
                result.sampledImageCount++;
            },
            [&](const Coral::CombinedImageSampler& imageAndSampler)
            {
                result.combinedImageSamplerCount++;
            }
        }, binding.descriptor);
    }

    return result;
}


std::optional<Coral::DescriptorSet::CreateError>
DescriptorSetImpl::init(const DescriptorSet::CreateConfig& config)
{
    auto device = context().getVkDevice();

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& binding : config.layout.bindings)
    {
        bindings.push_back({
            .binding            = binding.binding,
            .descriptorType     = convert(binding.type),
            .descriptorCount    = 1,//binding.count,
            .stageFlags         = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = VK_NULL_HANDLE,
        });
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo
    { 
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data(), 
    };

    VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mLayout) != VK_SUCCESS)
    {
        return Coral::DescriptorSet::CreateError::INTERNAL_ERROR;
    }

    // Count all descriptors so we can request the correctly sized DescriptorSet from the Pool.
    // Note that the size of the layout might not be identical to the actual size due to
    // descriptor indexing.
    DescriptorSetSize sizes = countDescriptors(config.bindings);

    mDescriptorSet = context().getDescriptorPool().allocateDescriptorSet(mLayout, sizes);
    if (mDescriptorSet == VK_NULL_HANDLE)
    {
        return Coral::DescriptorSet::CreateError::INTERNAL_ERROR;
    }
 
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(sizes.uniformBufferCount + sizes.storageBufferCount);
    std::vector<VkDescriptorImageInfo> imageInfos;
    bufferInfos.reserve(sizes.sampledImageCount + sizes.samplerCount + sizes.combinedImageSamplerCount);

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(bufferInfos.size() + imageInfos.size());
    for (const auto& binding : config.bindings)
    {
        auto& descriptorWrite = descriptorWrites.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        descriptorWrite.dstSet          = mDescriptorSet;
        descriptorWrite.dstBinding      = binding.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        
        std::visit(Coral::Visitor{
            [&](const Coral::BufferPtr& buffer)
            {
                auto impl = std::static_pointer_cast<BufferImpl>(buffer);
                bufferInfos.push_back({
                    .buffer = impl->getVkBuffer(),
                    .offset = 0,
                    .range = VK_WHOLE_SIZE,
                });
                
                if (impl->type() == CO_BUFFER_TYPE_STORAGE)
                {
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }
                else if (impl->type() == CO_BUFFER_TYPE_UNIFORM)
                {
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                }
                else
                {
                    assert(false);
                }
                descriptorWrite.pBufferInfo = &bufferInfos.back();
   
            },
            [&](const Coral::ImagePtr& image)
            {
                auto impl = std::static_pointer_cast<ImageImpl>(image);
                imageInfos.push_back({
                    .sampler     = VK_NULL_HANDLE,
                    .imageView   = impl->getVkImageView(),
                    .imageLayout = impl->getPreferredImageLayout()
                });

                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrite.pImageInfo = &imageInfos.back();
            },
            [&](const Coral::SamplerPtr& sampler)
            {
                auto impl = std::static_pointer_cast<SamplerImpl>(sampler);
                imageInfos.push_back({
                    .sampler     = impl->getVkSampler(),
                    .imageView   = VK_NULL_HANDLE,
                    .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
                });

                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                descriptorWrite.pImageInfo = &imageInfos.back();
            },
            [&](const Coral::CombinedImageSampler& imageAndSampler)
            {
                auto imageImpl   = std::static_pointer_cast<ImageImpl>(imageAndSampler.image);
                auto samplerImpl = std::static_pointer_cast<SamplerImpl>(imageAndSampler.sampler);

                imageInfos.push_back({
                    .sampler     = samplerImpl->getVkSampler(),
                    .imageView   = imageImpl->getVkImageView(),
                    .imageLayout = imageImpl->getPreferredImageLayout(),
                });

                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.pImageInfo     = &imageInfos.back();
            }
        }, binding.descriptor);
    }

    vkUpdateDescriptorSets(device,
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        /*descriptorCopyCount*/ 0,
        /*pDescriptorCopies*/ nullptr);

    return {};
}
