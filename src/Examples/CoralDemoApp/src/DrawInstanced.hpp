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

    bool initializeBindings();

    CoContext mContext;

    CoBuffer mCameraParams;
    CoBuffer mLightParams;

    Coral::ShaderModulePtr mVertexShader;
    Coral::ShaderModulePtr mFragmentShader;
    Coral::PipelineStatePtr mPipelineState;

    struct DrawBatch
    {
        Coral::BufferPtr instanceParamsBuffer;
        std::shared_ptr<const Util::Mesh> mesh;
        std::shared_ptr<const Util::Material> material;
        uint32_t instanceCount{ 0 };
        Coral::DescriptorSetPtr descriptorSet;
    };

    std::vector<DrawBatch> mDrawBatches;

    struct
    {
        uint32_t cameraParams;
        uint32_t instanceParams;
        uint32_t lightParams;
        uint32_t baseColorTexture;
    } mBindings;
}; // class DrawInstanced

#endif // !DRAWINSTANCED_HPP
