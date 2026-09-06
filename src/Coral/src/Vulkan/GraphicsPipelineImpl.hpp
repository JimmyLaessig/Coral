#ifndef CORAL_VULKAN_GRAPHICSPIPELINEIMPL_HPP
#define CORAL_VULKAN_GRAPHICSPIPELINEIMPL_HPP

#include "GraphicsPipeline.hpp"
#include "Resource.hpp"

#include <optional>
#include <span>

namespace Coral::Vulkan
{

/*!
 * Implementation of the GraphicsPipeline interface using the Vulkan backend
 */
class GraphicsPipelineImpl : public Coral::GraphicsPipeline
                        , public Resource
                        , public std::enable_shared_from_this<GraphicsPipelineImpl>
{
public:

    using Resource::Resource;

    virtual ~GraphicsPipelineImpl();

    std::optional<GraphicsPipeline::CreateError> init(const GraphicsPipeline::CreateConfig& config);

    VkPipeline getVkPipeline();

    VkPipelineLayout getVkPipelineLayout();

    VkPipelineBindPoint getVkPipelineBindingPoint() { return VK_PIPELINE_BIND_POINT_GRAPHICS; }

private:

    VkPipelineLayout mPipelineLayout{ VK_NULL_HANDLE };

    VkPipeline mPipeline{ VK_NULL_HANDLE };

}; // class GraphicsPipelineImpl

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_GRAPHICSPIPELINEIMPL_HPP
