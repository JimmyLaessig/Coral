#ifndef DRAWINDEXED_HPP
#define DRAWINDEXED_HPP

#include "Util.hpp"
#include "FlyCamera.hpp"

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <glm/glm.hpp>

struct RenderObject
{
    std::shared_ptr<const Util::Mesh> mesh;
    std::shared_ptr<const Util::Material> material;
    glm::mat4 modelMatrix;
}; // struct RenderObject

struct Scene
{
    const std::vector<RenderObject>& renderObjects;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 lightDirection;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
};

/*!
 *
 */
class DrawIndexed
{
public:

    bool initialize(CoContext context, const Scene& scene);

    void draw(CoCommandBuffer commandBuffer, const FlyCamera& camera);

    void drawOptimized(CoCommandBuffer commandBuffer, const FlyCamera& camera);

private:

    CoContext mContext;

    Coral::ShaderModulePtr mVertexShader;
    Coral::ShaderModulePtr mFragmentShader;

    Util::UniformBlockBuilder mCameraParamsBuilder;
    Coral::BufferPtr mCameraParams;
    Util::UniformBlockBuilder mLightParamsBuilder;
    Coral::BufferPtr mLightParams;

    Coral::GraphicsPipelinePtr mGraphicsPipeline;

    struct InstanceData
    {
        Coral::BufferPtr instanceParamsBuffer;
        Coral::DescriptorSetPtr descriptorSet;
    };

    struct DrawBatch
    {
        std::shared_ptr<const Util::Mesh> mesh;
        std::shared_ptr<const Util::Material> material;
        std::vector<InstanceData> instances;
    };

    std::vector<DrawBatch> mBatches;

}; // class DrawIndexed

#endif // !DRAWINDEXED_HPP
