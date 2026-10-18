#ifndef DRAWINDIRECT_HPP
#define DRAWINDIRECT_HPP

#include "Util.hpp"
#include "FlyCamera.hpp"

#include "DrawInstanced.hpp"

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <glm/glm.hpp>

/*!
 *
 */
class DrawIndirect
{
public:

    bool initialize(CoContext context, 
                    const Scene& scene);

    void draw(CoCommandBuffer commandBuffer, const FlyCamera& camera);

private:

    CoContext mContext;

    Util::UniformBlockBuilder mCameraParamsBuilder;
    Coral::BufferPtr mCameraParams;
    Util::UniformBlockBuilder mLightParamsBuilder;
    Coral::BufferPtr mLightParams;

    Coral::ShaderModulePtr mVertexShader;
    Coral::ShaderModulePtr mFragmentShader;
    Coral::GraphicsPipelinePtr mGraphicsPipeline;

    struct DrawBatch
    {
        Coral::BufferPtr indirectBuffer;
        Coral::BufferPtr instanceParamsBuffer;
        Coral::BufferPtr drawParamsBuffer;
        std::shared_ptr<const Util::Material> material;
        Coral::DescriptorSetPtr descriptorSet;
        uint32_t drawCount{ 0 };
    };

    std::vector<DrawBatch> mDrawBatches;

}; // class DrawIndirect

#endif // !DRAWINDIRECT_HPP
