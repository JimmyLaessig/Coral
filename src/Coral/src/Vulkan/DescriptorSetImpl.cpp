#include "DescriptorSetImpl.hpp"

#include "DescriptorSetPool.hpp"
#include "BufferImpl.hpp"
#include "SamplerImpl.hpp"
#include "ImageImpl.hpp"
#include "../Visitor.hpp"

#include <variant>
#include <vector>

using namespace Coral::Vulkan;


DescriptorSetImpl::~DescriptorSetImpl()
{
	if (mContext)
	{
		mContext->getDescriptorSetPool().freeDescriptorSet(mDescriptorSet);

		if (mDescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(mContext->getVkDevice(), mDescriptorSetLayout, nullptr);
		}
	}
}


VkDescriptorSet
DescriptorSetImpl::getVkDescriptorSet()
{
	return mDescriptorSet;
}


std::optional<Coral::DescriptorSetCreationError>
DescriptorSetImpl::init(Coral::Vulkan::ContextImpl& context, const Coral::DescriptorSetCreateConfig& config)
{
	mContext = &context;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (const auto& binding : config.bindings)
	{
		auto& vkBinding				= bindings.emplace_back();
		vkBinding.binding			= binding.binding;
		vkBinding.descriptorCount	= 1;
		vkBinding.stageFlags		= VK_SHADER_STAGE_ALL_GRAPHICS;
		vkBinding.descriptorType	= std::visit(Coral::Visitor{
			[](const Coral::Buffer*) -> VkDescriptorType { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; },
			[](const Coral::Sampler*) -> VkDescriptorType { return VK_DESCRIPTOR_TYPE_SAMPLER; },
			[](const Coral::Image*) -> VkDescriptorType { return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; },
			[](const Coral::CombinedTextureSampler&) -> VkDescriptorType { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
		}, binding.descriptor);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings	= bindings.data();

	if (vkCreateDescriptorSetLayout(mContext->getVkDevice(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
	{
		return Coral::DescriptorSetCreationError::INTERNAL_ERROR;
	}
	 
	mDescriptorSet = mContext->getDescriptorSetPool().allocateDescriptorSet(mDescriptorSetLayout);

	if (mDescriptorSet == VK_NULL_HANDLE)
	{
		return Coral::DescriptorSetCreationError::INTERNAL_ERROR;
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites(config.bindings.size());
	std::vector<std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>> descriptorInfos(config.bindings.size());

	size_t i{ 0 };
	for (const auto& binding : config.bindings)
	{
		auto& descriptorWrite		= descriptorWrites[i];
		descriptorWrite.sType		= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet		= mDescriptorSet;
		descriptorWrite.dstBinding	= binding.binding;

		std::visit(Coral::Visitor{
			[&](Coral::Buffer* buffer)
			{ 
				auto bufferImpl		= static_cast<Coral::Vulkan::BufferImpl*>(buffer);

				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer	= bufferImpl->getVkBuffer();
				bufferInfo.offset	= 0;
				bufferInfo.range	= buffer->size();
				descriptorInfos[i]	= bufferInfo;

				descriptorWrite.pBufferInfo		= &std::get<VkDescriptorBufferInfo>(descriptorInfos[i]);
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
			},
			[&](Coral::Sampler* sampler)
			{
				auto samplerImpl = static_cast<Coral::Vulkan::SamplerImpl*>(sampler);

				VkDescriptorImageInfo info;
				info.sampler	   = samplerImpl->getVkSampler();
				info.imageView	   = nullptr;
				descriptorInfos[i] = info;

				descriptorWrite.pImageInfo		= &std::get<VkDescriptorImageInfo>(descriptorInfos[i]);
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
			},
			[&](Coral::Image* image)
			{
				auto imageImpl = static_cast<Coral::Vulkan::ImageImpl*>(image);

				VkDescriptorImageInfo info;
				info.sampler = nullptr;
				info.imageView = imageImpl->getVkImageView();
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//  image->getVkImageLayout();
				descriptorInfos[i] = info;

				descriptorWrite.pImageInfo = &std::get<VkDescriptorImageInfo>(descriptorInfos[i]);
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorWrite.descriptorCount = 1;
			},
			[&](Coral::CombinedTextureSampler descriptor)
			{
				auto imageImpl = static_cast<Coral::Vulkan::ImageImpl*>(descriptor.texture);
				auto samplerImpl = static_cast<Coral::Vulkan::SamplerImpl*>(descriptor.sampler);

				VkDescriptorImageInfo info;
				info.sampler = samplerImpl->getVkSampler();
				info.imageView = imageImpl->getVkImageView();
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//  image->getVkImageLayout();
				descriptorInfos[i] = info;

				descriptorWrite.pImageInfo = &std::get<VkDescriptorImageInfo>(descriptorInfos[i]);
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
			}
		}, binding.descriptor);

		i++;
	}

	vkUpdateDescriptorSets(mContext->getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	return {};
}
