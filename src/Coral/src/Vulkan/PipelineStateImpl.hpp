#ifndef CORAL_VULKAN_PIPELINESTATEIMPL_HPP
#define CORAL_VULKAN_PIPELINESTATEIMPL_HPP

#include <Coral/PipelineState.hpp>

#include "ContextImpl.hpp"

#include <optional>
#include <span>
#include <vector>

namespace Coral::Vulkan
{

class PipelineStateImpl : public Coral::PipelineState
{
public:

	virtual ~PipelineStateImpl();

	std::optional<PipelineStateCreationError> init(Coral::Vulkan::ContextImpl& context, const PipelineStateCreateConfig& config);

	VkPipeline getVkPipeline();

	std::span<VkDescriptorSetLayout> getVkDescriptorSetLayouts();

	VkPipelineLayout getVkPipelineLayout();

	VkPipelineBindPoint getVkPipelineBindingPoint() { return VK_PIPELINE_BIND_POINT_GRAPHICS; }

private:

	ContextImpl* mContext{ nullptr };

	VkPipelineLayout mPipelineLayout{ VK_NULL_HANDLE };

	VkPipeline mPipeline{ VK_NULL_HANDLE };

	VkDescriptorSetLayout mDescriptorSetLayout;

	FaceCullingMode mFaceCullingMode{ FaceCullingModes::BackFaceCulling };

	DepthTestMode mDepthTestMode{ DepthTestModes::Less };

	BlendMode mBlendMode{ BlendModes::Blend };

	PolygonMode mPolygonMode{ PolygonMode::SOLID };

	std::optional<StencilTestMode> mStencilTestMode;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_PIPELINESTATEIMPL_HPP