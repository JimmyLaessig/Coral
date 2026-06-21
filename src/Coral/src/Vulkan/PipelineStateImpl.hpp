#ifndef CORAL_VULKAN_PIPELINESTATEIMPL_HPP
#define CORAL_VULKAN_PIPELINESTATEIMPL_HPP

#include "PipelineState.hpp"

#include "Fwd.hpp"
#include "Resource.hpp"
#include "Vulkan.hpp"

#include <optional>
#include <span>
#include <vector>

namespace Coral::Vulkan
{

/*!
 * Implementation of the PipelineState interface using the Vulkan backend
 */
class PipelineStateImpl : public Coral::PipelineState
                        , public Resource
                        , public std::enable_shared_from_this<PipelineStateImpl>
{
public:

    using Resource::Resource;

    virtual ~PipelineStateImpl();

    std::optional<PipelineState::CreateError> init(const PipelineState::CreateConfig& config);

    VkPipeline getVkPipeline();

    std::span<VkDescriptorSetLayout> getVkDescriptorSetLayouts();

    VkPipelineLayout getVkPipelineLayout();

    VkPipelineBindPoint getVkPipelineBindingPoint() { return VK_PIPELINE_BIND_POINT_GRAPHICS; }

private:

    VkPipelineLayout mPipelineLayout{ VK_NULL_HANDLE };

    VkPipeline mPipeline{ VK_NULL_HANDLE };

    VkDescriptorSetLayout mDescriptorSetLayout{ VK_NULL_HANDLE };

    CoFaceCullingMode mFaceCullingMode;

    CoDepthTestMode mDepthTestMode;

    CoBlendMode mBlendMode;

    CoPolygonMode mPolygonMode;

    std::optional<CoStencilTestMode> mStencilTestMode;

}; // class PipelineStateImpl

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_PIPELINESTATEIMPL_HPP
