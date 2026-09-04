
#include "DescriptorSetImpl.hpp"

#include "BufferImpl.hpp"
#include "ContextImpl.hpp"
#include "ImageImpl.hpp"
#include "SamplerImpl.hpp"

#include <algorithm>
#include <vector>
#include <span>

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
    if (mDescriptorSet != VK_NULL_HANDLE)
    {
        context().getDescriptorPool().freeDescriptorSet(mLayout, mDescriptorSet);
    }
}


void
reserveDescriptorWrites(const std::span<CoDescriptorBinding>& bindings, 
                       std::vector<VkDescriptorBufferInfo>& bufferInfos, 
                       std::vector<VkDescriptorImageInfo>& imageInfos,
                       std::vector<VkWriteDescriptorSet>& descriptorWrites)
{
    auto bufferInfoCount = std::count_if(bindings.begin(), bindings.end(), [](const CoDescriptorBinding& binding)
    {
            return binding.type == CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER || 
                   binding.type == CO_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    });

    auto imageInfoCount = std::count_if(bindings.begin(), bindings.end(), [](const CoDescriptorBinding& binding)
    {
        return binding.type == CO_DESCRIPTOR_TYPE_IMAGE ||
               binding.type == CO_DESCRIPTOR_TYPE_SAMPLER ||
               binding.type == CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    });

    bufferInfos.reserve(bufferInfoCount);
    imageInfos.reserve(imageInfoCount);
    descriptorWrites.reserve(bufferInfoCount + imageInfoCount);
}


uint32_t
countImageInfos(const std::span<CoDescriptorBinding>& bindings)
{
    auto result = std::ranges::count_if(bindings, [](const CoDescriptorBinding& binding)
        {
            return binding.type == CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                binding.type == CO_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        });

    return static_cast<uint32_t>(result);
}


std::optional<Coral::DescriptorSet::CreateError>
DescriptorSetImpl::init(const DescriptorSet::CreateConfig& config)
{
    std::tie(mLayout, mDescriptorSet) = context().getDescriptorPool().allocateDescriptorSet(config);
    if (mLayout == VK_NULL_HANDLE ||
        mDescriptorSet == VK_NULL_HANDLE)
    {
        return Coral::DescriptorSet::CreateError::INTERNAL_ERROR;
    }

    auto device = context().getVkDevice();

    std::span bindings(config.pDescriptorBindings, config.descriptorBindingCount);

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    reserveDescriptorWrites(bindings, bufferInfos, imageInfos, descriptorWrites);

    for (const auto& binding : bindings)
    {
        auto& descriptorWrite = descriptorWrites.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        descriptorWrite.dstSet          = mDescriptorSet;
        descriptorWrite.dstBinding      = binding.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = ::convert(binding.type);

        VkDescriptorBufferInfo bufferInfo{};
        VkDescriptorImageInfo imageInfo{};

        switch (binding.type)
        {
            case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {
                auto impl = std::static_pointer_cast<BufferImpl>(binding.buffer->impl);

                bufferInfos.push_back({
                    .buffer = impl->getVkBuffer(),
                    .offset = 0,
                    .range = VK_WHOLE_SIZE,
                });

                descriptorWrite.pBufferInfo     = &bufferInfos.back();
                descriptorWrite.descriptorCount = 1;
                break;
            }
            case CO_DESCRIPTOR_TYPE_IMAGE:
            {
                auto impl = std::static_pointer_cast<ImageImpl>(binding.image->impl);

                imageInfos.push_back({
                    .sampler     = VK_NULL_HANDLE,
                    .imageView   = impl->getVkImageView(),
                    .imageLayout = impl->getPreferredImageLayout(),
                });

                descriptorWrite.pImageInfo      = &imageInfos.back();
                descriptorWrite.descriptorCount = 1;

                break;
            }
            case CO_DESCRIPTOR_TYPE_SAMPLER:
            {
                auto impl = std::static_pointer_cast<SamplerImpl>(binding.sampler->impl);

                imageInfos.push_back({
                    .sampler   = impl->getVkSampler(),
                    .imageView = VK_NULL_HANDLE,
                });

                descriptorWrite.pImageInfo      = &imageInfos.back();
                descriptorWrite.descriptorCount = 1;

                break;
            }
            case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            {
                auto imageImpl   = std::static_pointer_cast<ImageImpl>(binding.combinedImageSampler.image->impl);
                auto samplerImpl = std::static_pointer_cast<SamplerImpl>(binding.combinedImageSampler.sampler->impl);

                imageInfos.push_back({
                    .sampler     = samplerImpl->getVkSampler(),
                    .imageView   = imageImpl->getVkImageView(),
                    .imageLayout = imageImpl->getPreferredImageLayout(),
                });

                descriptorWrite.pImageInfo      = &imageInfos.back();
                descriptorWrite.descriptorCount = 1;

                break;
            }
        }
    }

    vkUpdateDescriptorSets(device,
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        /*descriptorCopyCount*/ 0,
        /*pDescriptorCopies*/ nullptr);

    return {};
}