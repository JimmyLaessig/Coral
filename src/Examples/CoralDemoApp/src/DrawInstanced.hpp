#ifndef DRAWINSTANCED_HPP
#define DRAWINSTANCED_HPP

#include "Util.hpp"

#include "DrawIndexed.hpp"

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <glm/glm.hpp>

/*!
 *
 */
class DrawInstanced
{
public:

    bool initialize(CoContext context, 
                    const Scene& scene);

    void draw(CoCommandBuffer commandBuffer);

private:

    CoContext mContext;

    CoBuffer mCameraParams;
    CoBuffer mLightParams;

    Coral::ShaderModulePtr mVertexShader;
    Coral::ShaderModulePtr mFragmentShader;
    Coral::GraphicsPipelinePtr mGraphicsPipeline;

    struct DrawBatch
    {
        Coral::BufferPtr instanceParamsBuffer;
        std::shared_ptr<const Util::Mesh> mesh;
        std::shared_ptr<const Util::Material> material;
        uint32_t instanceCount{ 0 };
        Coral::DescriptorSetPtr descriptorSet;
    };

    std::vector<DrawBatch> mDrawBatches;

}; // class DrawInstanced

#endif // !DRAWINSTANCED_HPP
