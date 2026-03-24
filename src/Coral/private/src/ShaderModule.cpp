#include <Coral/ShaderModule.h>
#include <Coral/ShaderModule.hpp>

#include <Coral/Context.hpp>

#include <Coral/Visitor.hpp>

using namespace Coral;


CoResult
coContextCreateShaderModule(CoContext context,
	                        const CoShaderModuleCreateConfig* pConfig,
	                        CoShaderModule* pShaderModule)
{
	Coral::ShaderModule::CreateConfig internalConfig{};
	internalConfig.entryPoint = pConfig->pEntryPoint;
	internalConfig.name       = pConfig->pName;
	internalConfig.source     = std::as_bytes(std::span(pConfig->pSource, pConfig->sourceCount));
	internalConfig.stage      = pConfig->stage;
	auto impl = context->impl->createShaderModule(internalConfig);
	if (impl)
	{
		*pShaderModule = new CoShaderModule_T{ impl.value() };
		return CO_SUCCESS;
	}

	return static_cast<CoResult>(impl.error());
}


void
coDestroyShaderModule(CoShaderModule shaderModule)
{
	delete shaderModule;
}


void
coShaderModuleGetLayout(const CoShaderModule shaderModule, CoShaderModuleLayout* pLayout)
{
	if (shaderModule->mInputAttributeLayoutData.empty())
	{
		for (const auto& binding : shaderModule->impl->inputAttributeLayout())
		{
			shaderModule->mInputAttributeLayoutData.emplace_back(
				binding.binding, 
				binding.location, 
				binding.format, 
				shaderModule->mStringCache.insert(binding.name).first->c_str()
			);
		}
	}

	if (shaderModule->mOutputAttributeLayoutData.empty())
	{
		for (const auto& binding : shaderModule->impl->outputAttributeLayout())
		{
			shaderModule->mOutputAttributeLayoutData.emplace_back(
				binding.binding,
				binding.location,
				binding.format,
				shaderModule->mStringCache.insert(binding.name).first->c_str()
			);
		}
	}

	if (shaderModule->mDescriptorBindingInfos.empty())
	{
		auto layout = shaderModule->impl->descriptorLayout();
		// Begin by allocating the space for all member definitions
		for (const auto& descriptor : layout)
		{
			std::visit(Visitor{
				[&](auto) {},
				[&](const UniformBlockDefinition& block)
				{
					for (const auto& member : block.members)
					{
						auto& coMember = shaderModule->mMembersCache.emplace_back();
						coMember.count = member.count;
						coMember.pName = shaderModule->mStringCache.insert(member.name).first->c_str();
						coMember.size  = member.paddedSize;
						coMember.type  = member.type;
					}
				}
			}, descriptor.definition);
		}

		size_t memberOffset{ 0 };
		for (const auto& descriptor : layout)
		{
			auto& info = shaderModule->mDescriptorBindingInfos.emplace_back();
			info.binding = descriptor.binding;
			info.pName = shaderModule->mStringCache.insert(descriptor.name).first->c_str();

			std::visit(Visitor{
				[&](SamplerDefinition)              { info.type = CO_DESCRIPTOR_TYPE_SAMPLER; },
				[&](TextureDefinition)              { info.type = CO_DESCRIPTOR_TYPE_TEXTURE; },
				[&](CombinedTextureSamplerDefinition) { info.type = CO_DESCRIPTOR_TYPE_COMBINED_TEXTURE_SAMPLER; },
				[&](const UniformBlockDefinition& block)
				{
					info.type               = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					info.block.pMembers    = shaderModule->mMembersCache.data() + memberOffset;
					info.block.memberCount = static_cast<uint32_t>(block.members.size());
					memberOffset += block.members.size();
				}
			}, descriptor.definition);
		}
	}

	pLayout->pInputAttributeBindingInfos     = shaderModule->mInputAttributeLayoutData.data();
	pLayout->inputAttributeBindingInfoCount  = static_cast<uint32_t>(shaderModule->mInputAttributeLayoutData.size());
	pLayout->pOutputAttributeBindingInfos    = shaderModule->mOutputAttributeLayoutData.data();
	pLayout->outputAttributeBindingInfoCount = static_cast<uint32_t>(shaderModule->mOutputAttributeLayoutData.size());
	pLayout->pDescriptorBindingInfos         = shaderModule->mDescriptorBindingInfos.data();
	pLayout->descriptorBindingInfoCount      = static_cast<uint32_t>(shaderModule->mDescriptorBindingInfos.size());
}













